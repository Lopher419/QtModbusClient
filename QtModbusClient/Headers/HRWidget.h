#pragma once

#include <QWidget>
#include <QModbusReply>
#include <QPointer>
#include <QStandardItemModel>
#include "ui_HRWidget.h"
#include "ModbusConnection.h"

class HRWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HRWidget(QWidget* parent = nullptr);
    ~HRWidget();

    void setModbusConnection(ModbusConnection* connection);

private slots:
    void onReadSingleHR();
    void handleSingleHRReadResult();
    void onWriteSingleHR();
    void handleSingleHRWriteResult();
    void onReadMultipleHR();
    void handleMultipleHRReadResult();
    void onWriteMultipleHR();
    void handleMultipleHRWriteResult(); // 确保声明正确

private:
    void initUI();
    void initTableModels();
    void safeDeleteReply(QModbusReply* reply);
    void updateWriteTable();
    void handleWriteItemChanged(QStandardItem* item);
    void setupConnections();

    Ui::HRWidget ui;

    ModbusConnection* m_modbusConnection = nullptr;
    QStandardItemModel* m_hrReadModel = nullptr;
    QStandardItemModel* m_hrWriteModel = nullptr;

    QPointer<QModbusReply> m_singleHRReadReply;
    QPointer<QModbusReply> m_singleHRWriteReply;
    QPointer<QModbusReply> m_multipleHRReadReply;
    QPointer<QModbusReply> m_multipleHRWriteReply;
};