#pragma once

#include <QObject>
#include <QModbusRtuSerialMaster>
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

    QModbusRtuSerialMaster* master() const { return m_modbusMaster; }

signals:
    void connectionStateChanged(bool connected);

private:
    QModbusRtuSerialMaster* m_modbusMaster = nullptr;
};