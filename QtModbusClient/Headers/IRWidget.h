#pragma once

#include <QWidget>
#include <QModbusReply>
#include <QStandardItemModel>
#include "ui_IRWidget.h"
#include "ModbusConnection.h"

class IRWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IRWidget(QWidget* parent = nullptr);
    ~IRWidget();

    void setModbusConnection(ModbusConnection* connection);

private slots:
    void onReadSingleIR();
    void handleSingleIRReadResult();
    void onReadMultipleIR();
    void handleMultipleIRReadResult();

private:
    void initUI();
    void initTableModel();
    void safeDeleteReply(QModbusReply* reply);
    void processSingleIRResult(QModbusReply* reply);
    void processMultipleIRResult(QModbusReply* reply);
    void setupConnections();
    void updateHexColumn(const QVector<quint16>& values, int startAddr);

    Ui::IRWidget ui;
    ModbusConnection* m_modbusConnection = nullptr;
    QStandardItemModel* m_irModel = nullptr;
    QModbusReply* m_singleIRReadReply = nullptr;
    QModbusReply* m_multipleIRReadReply = nullptr;
};