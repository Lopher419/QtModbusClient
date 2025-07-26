#pragma once

#include <QObject>
#include <QModbusRtuSerialClient>
#include <QModbusDevice>

class ModbusManager : public QObject
{
    Q_OBJECT
public:
    explicit ModbusManager(QObject* parent = nullptr);
    ~ModbusManager();

    bool connectDevice();
    void disconnectDevice();
    bool isConnected() const;

    QModbusRtuSerialClient* master() const { return m_modbusMaster; }

signals:
    void connectionStateChanged(bool connected);

private:
    QModbusRtuSerialClient* m_modbusMaster = nullptr;
};