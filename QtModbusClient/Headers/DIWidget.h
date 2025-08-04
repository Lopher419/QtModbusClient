// DIWidget.h
#pragma once

#include <QWidget>
#include "ui_DIWidget.h"
#include "ModbusConnection.h"

class QStandardItemModel;
class QModbusReply;

class DIWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DIWidget(QWidget* parent = nullptr);
    ~DIWidget();

    void setModbusConnection(ModbusConnection* connection);

private slots:
    void onReadSingleDI();
    void onReadMultipleDI();

private:
    void handleSingleDIReadResult();
    void handleMultipleDIReadResult();
    void initUI();
    void initTableModel();
    void safeDeleteReply(QModbusReply* reply);
    void processSingleDIResult(QModbusReply* reply);
    void processMultipleDIResult(QModbusReply* reply);
    void setupConnections();
    void updateHexColumn(const QVector<quint8>& bytes, int byteCount);

    Ui::DIWidget ui;
    ModbusConnection* m_modbusConnection = nullptr;
    QStandardItemModel* m_diModel = nullptr;
    QModbusReply* m_singleDIReadReply = nullptr;
    QModbusReply* m_multipleDIReadReply = nullptr;
};