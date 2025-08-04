#pragma once

#include <QObject>
#include <QModbusRtuSerialClient>
#include <QSerialPort>
#include <QMutex>
#include <QPointer>

class ModbusConnection : public QObject
{
    Q_OBJECT

public:
    enum RegisterType {
        Coils = QModbusDataUnit::Coils,
        DiscreteInputs = QModbusDataUnit::DiscreteInputs,
        InputRegisters = QModbusDataUnit::InputRegisters,
        HoldingRegisters = QModbusDataUnit::HoldingRegisters
    };
    Q_ENUM(RegisterType)
        
        explicit ModbusConnection(QObject* parent = nullptr);
    ~ModbusConnection();

	// Connect to a Modbus device using the specified parameters
    void connectToDevice(const QString& portName,
        qint32 baudRate,
        QSerialPort::DataBits dataBits,
        QSerialPort::Parity parity,
        QSerialPort::StopBits stopBits,
        int slaveID
    );
    void closeConnection();
    bool isConnected() const;

	// Getters for connection parameters
    QString getPortName() const noexcept;
    qint32 getBaudRate() const noexcept;
    int getSlaveID() const noexcept;

	//Modbus operations
	QModbusReply* readRegister(RegisterType type, int startAddr, quint16 count);
    QModbusReply* writeCoil(int addr, bool value);
    QModbusReply* writeSingleRegister(int addr, quint16 value);
    QModbusReply* writeMultipleRegisters(RegisterType type, int startAddr, const QVector<quint16>& values);

signals:
    void connectionOpened();
    void connectionError(const QString& errorMessage);
    void connectionClosed();

private slots:
    void handleStateChanged(QModbusDevice::State state);
    void handleErrorOccurred(QModbusDevice::Error error);

private:
    QModbusRtuSerialClient* m_client = nullptr;
	mutable QMutex m_mutex; // Mutex for thread safety

	// Connection parameters
    QString m_port;
    qint32 m_baud;
    QSerialPort::DataBits m_dataBits;
    QSerialPort::Parity m_parity;
    QSerialPort::StopBits m_stopBits;
    int m_slaveID = 1;
};