#include "ModbusManager.h"

ModbusManager::ModbusManager(QObject* parent) : QObject(parent) {}

ModbusManager::~ModbusManager()
{
    disconnectDevice();
}

bool ModbusManager::connectSerial(
    const QString& portName,
    QSerialPort::BaudRate baudRate,
    QSerialPort::DataBits dataBits,
    QSerialPort::StopBits stopBits,
    QSerialPort::Parity parity,
    QSerialPort::FlowControl flowControl)
{
    if (m_modbusDevice) {
        disconnectDevice();
        delete m_modbusDevice;
    }

    m_modbusDevice = new QModbusRtuSerialMaster(this);

    m_modbusDevice->setConnectionParameter(
        QModbusDevice::SerialPortNameParameter, portName);
    m_modbusDevice->setConnectionParameter(
        QModbusDevice::SerialBaudRateParameter, baudRate);
    m_modbusDevice->setConnectionParameter(
        QModbusDevice::SerialDataBitsParameter, dataBits);
    m_modbusDevice->setConnectionParameter(
        QModbusDevice::SerialStopBitsParameter, stopBits);
    m_modbusDevice->setConnectionParameter(
        QModbusDevice::SerialParityParameter, parity);
    m_modbusDevice->setConnectionParameter(
        QModbusDevice::SerialFlowControlParameter, flowControl);

    m_modbusDevice->setTimeout(1000);
    m_modbusDevice->setNumberOfRetries(3);

    if (!m_modbusDevice->connectDevice()) {
        emit errorOccurred(tr("Á¬½ÓÊ§°Ü: %1").arg(m_modbusDevice->errorString()));
        return false;
    }

    connect(m_modbusDevice, &QModbusDevice::stateChanged, [this](int state) {
        emit connectionStateChanged(state == QModbusDevice::ConnectedState);
        });

    return true;
}

void ModbusManager::disconnectDevice()
{
    if (m_modbusDevice && m_modbusDevice->state() == QModbusDevice::ConnectedState) {
        m_modbusDevice->disconnectDevice();
    }
}

bool ModbusManager::isConnected() const
{
    return m_modbusDevice &&
        m_modbusDevice->state() == QModbusDevice::ConnectedState;
}
