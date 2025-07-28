#pragma once

#include <QObject>
#include <QModbusRtuSerialMaster>
#include <QModbusDataUnit>
#include <QSerialPort>

class ModbusManager : public QObject
{
    Q_OBJECT
public:
    explicit ModbusManager(QObject* parent = nullptr);
    ~ModbusManager();

    bool connectSerial(
        const QString& portName,
        QSerialPort::BaudRate baudRate,
        QSerialPort::DataBits dataBits,
        QSerialPort::StopBits stopBits,
        QSerialPort::Parity parity,
        QSerialPort::FlowControl flowControl);

    void disconnectDevice();
    bool isConnected() const;

signals:
    void connectionStateChanged(bool connected);
    void errorOccurred(const QString& message);

private:
    QModbusClient* m_modbusDevice = nullptr;
};