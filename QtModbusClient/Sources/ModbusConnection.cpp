#include "ModbusConnection.h"
#include <QDebug>
#include <QVariant>
#include <QMutexLocker>
#include <qmessagebox.h>
#include <qdatastream.h>

ModbusConnection::ModbusConnection(QObject* parent)
    : QObject(parent),
    m_baud(QSerialPort::Baud9600),
    m_dataBits(QSerialPort::Data8),
    m_parity(QSerialPort::NoParity),
    m_stopBits(QSerialPort::OneStop)
{
}

ModbusConnection::~ModbusConnection()
{
    closeConnection();
}

// Connection management
void ModbusConnection::connectToDevice(const QString& port,
    qint32 baudRate,
    QSerialPort::DataBits dataBits,
    QSerialPort::Parity parity,
    QSerialPort::StopBits stopBits,
    int slaveID)
{
    m_slaveID = slaveID;

    if (isConnected()) {
        closeConnection();
    }

    if (!m_client) {
        m_client = new QModbusRtuSerialClient(this);
        connect(m_client, &QModbusClient::stateChanged,
            this, &ModbusConnection::handleStateChanged);
        connect(m_client, &QModbusClient::errorOccurred,
            this, &ModbusConnection::handleErrorOccurred);
    }

    // reset device state
    m_port = port;
    m_baud = baudRate;
    m_dataBits = dataBits;
    m_parity = parity;
    m_stopBits = stopBits;

    // configure client parameters
    m_client->setConnectionParameter(QModbusDevice::SerialPortNameParameter, QVariant(port));
    m_client->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, QVariant(baudRate));
    m_client->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, QVariant(dataBits));
    m_client->setConnectionParameter(QModbusDevice::SerialParityParameter, QVariant(parity));
    m_client->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, QVariant(stopBits));

    // configure additional parameters
    m_client->setTimeout(1000);
    m_client->setNumberOfRetries(1);

    // try to connect
    if (!m_client->connectDevice()) {
        QString error = tr("Connect fail : ") + m_client->errorString();
        emit connectionError(error);
    }
}

void ModbusConnection::closeConnection()
{
    if (m_client) {
        if (m_client->state() != QModbusDevice::UnconnectedState) {
            m_client->disconnectDevice();
        }
    }
}

bool ModbusConnection::isConnected() const
{
    return m_client && m_client->state() == QModbusDevice::ConnectedState;
}

QString ModbusConnection::getPortName() const noexcept
{
    return m_port;
}

qint32 ModbusConnection::getBaudRate() const noexcept
{
    return m_baud;
}

int ModbusConnection::getSlaveID() const noexcept
{
    return m_slaveID;
}

// Modbus operations
QModbusReply* ModbusConnection::readRegister(RegisterType type, int startAddr, quint16 count)
{
    QMutexLocker locker(&m_mutex);

    qDebug() << "\n[Modbus Read Request]";
    qDebug() << "Type:" << type
        << "| Start Addr:" << startAddr
        << "| Count:" << count
        << "| Slave ID:" << m_slaveID;

    if (!isConnected() || !m_client) {
        qWarning() << "Cannot read - not connected";
        return nullptr;
    }

    QModbusDataUnit request(static_cast<QModbusDataUnit::RegisterType>(type), startAddr, count);
    return m_client->sendReadRequest(request, m_slaveID);
}

// Write single coil value
QModbusReply* ModbusConnection::writeCoil(int addr, bool value)
{
    QMutexLocker locker(&m_mutex);

    if (!isConnected()) {
        qWarning() << "Write coil failed: Not connected";
        return nullptr;
    }

    QByteArray requestData;
    QDataStream stream(&requestData, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << static_cast<quint16>(addr);
    stream << static_cast<quint16>(value ? 0xFF00 : 0x0000);

    QModbusRequest request(QModbusRequest::WriteSingleCoil, requestData);
    return m_client->sendRawRequest(request, m_slaveID);
}

// Write single holding register
QModbusReply* ModbusConnection::writeSingleRegister(int addr, quint16 value)
{
    QMutexLocker locker(&m_mutex);

    if (!isConnected() || !m_client) {
        qWarning() << "Write single register failed: Not connected";
        return nullptr;
    }

    QByteArray requestData;
    QDataStream stream(&requestData, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << static_cast<quint16>(addr);
    stream << static_cast<quint16>(value);

    QModbusRequest request(QModbusRequest::WriteSingleRegister, requestData);

    qDebug() << "\n[Modbus WriteSingleRegister Request]";
    qDebug() << "Address:" << addr
        << "| Value:" << value
        << "| Slave ID:" << m_slaveID;

    return m_client->sendRawRequest(request, m_slaveID);
}

// Write multiple registers
QModbusReply* ModbusConnection::writeMultipleRegisters(RegisterType type, int startAddr, const QVector<quint16>& values)
{
    QMutexLocker locker(&m_mutex);
    if (!isConnected() || !m_client) {
        qWarning() << "Cannot write multiple registers - not connected";
        return nullptr;
    }

    if (type != Coils && type != HoldingRegisters) {
        qWarning() << "Invalid register type for multiple write";
        return nullptr;
    }

    QModbusDataUnit request(static_cast<QModbusDataUnit::RegisterType>(type), startAddr, values);

    qDebug() << "\n[Modbus WriteMultipleRegisters Request]";
    qDebug() << "Type:" << type
        << "| Start Addr:" << startAddr
        << "| Count:" << values.size()
        << "| Slave ID:" << m_slaveID;

    return m_client->sendWriteRequest(request, m_slaveID);
}

// modbus state change handling
void ModbusConnection::handleStateChanged(QModbusDevice::State state)
{
    qDebug() << "Modbus state changed:" << state;

    switch (state) {
    case QModbusDevice::ConnectedState:
        emit connectionOpened();
        break;
    case QModbusDevice::UnconnectedState:
        emit connectionClosed();
        break;
    default: break;
    }
}

// error handling
void ModbusConnection::handleErrorOccurred(QModbusDevice::Error error)
{
    if (error == QModbusDevice::NoError)
        return;

    QString errorMsg;
    switch (error) {
    case QModbusDevice::ConnectionError:
        errorMsg = tr("Connection error");
        break;
    case QModbusDevice::TimeoutError:
        errorMsg = tr("Response timeout");
        break;
    case QModbusDevice::ProtocolError:
        errorMsg = tr("Protocol error");
        break;
    case QModbusDevice::ReplyAbortedError:
        errorMsg = tr("Request aborted");
        break;
    case QModbusDevice::UnknownError:
    default:
        errorMsg = tr("Unknown error");
    }

    if (!m_client->errorString().isEmpty()) {
        errorMsg += ": " + m_client->errorString();
    }

    qWarning() << "Modbus error:" << errorMsg;
    emit connectionError(errorMsg);
}