#include "modbusmanager.h"
#include <QSerialPort>

ModbusManager::ModbusManager(QObject* parent)
    : QObject(parent)
{
    m_modbusMaster = new QModbusRtuSerialMaster(this);

    // 可以设置主站的串口参数，这里默认COM3，8N1，9600波特率
    m_modbusMaster->setConnectionParameter(QModbusDevice::SerialPortNameParameter, "COM3");
    m_modbusMaster->setConnectionParameter(QModbusDevice::SerialParityParameter, QSerialPort::EvenParity);
    m_modbusMaster->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, QSerialPort::Baud9600);
    m_modbusMaster->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, QSerialPort::Data8);
    m_modbusMaster->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, QSerialPort::OneStop);

    m_modbusMaster->setTimeout(1000);
    m_modbusMaster->setNumberOfRetries(3);

    connect(m_modbusMaster, &QModbusClient::stateChanged, this, [this](QModbusDevice::State state) {
        emit connectionStateChanged(state == QModbusDevice::ConnectedState);
        });
}

ModbusManager::~ModbusManager()
{
    if (m_modbusMaster) {
        m_modbusMaster->disconnectDevice();
        m_modbusMaster->deleteLater();
    }
}

bool ModbusManager::connectDevice()
{
    if (!m_modbusMaster)
        return false;

    if (m_modbusMaster->state() == QModbusDevice::ConnectedState)
        return true;

    return m_modbusMaster->connectDevice();
}

void ModbusManager::disconnectDevice()
{
    if (m_modbusMaster)
        m_modbusMaster->disconnectDevice();
}

bool ModbusManager::isConnected() const
{
    return m_modbusMaster && m_modbusMaster->state() == QModbusDevice::ConnectedState;
}