#pragma once

#include <QWidget>
#include <QModbusReply>
#include <QStandardItemModel>
#include "ui_CoilWidget.h"
#include "ModbusConnection.h"

class CoilWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CoilWidget(QWidget* parent = nullptr);
    ~CoilWidget();

    void setModbusConnection(ModbusConnection* connection);

private slots:
    void onReadCoil();
    void handleCoilReadResult();
    void onWriteCoil();
    void handleCoilWriteResult();

    void onReadMultipleCoils();
    void handleMultipleCoilsReadResult();
    void onWriteMultipleCoils();
    void handleMultipleCoilsWriteResult();

    void onSelectAllChanged(int state);

private:
    void initSingleCoilUI();
    void initMultipleCoilsReadUI();
    void initMultipleCoilsWriteUI();
    void initTableModels();

    void updateCoilsWriteTable();
    void handleCoilWriteItemChanged(QStandardItem* item);

    Ui::CoilWidget ui;

    ModbusConnection* m_modbusConnection = nullptr;

    QModbusReply* m_currentReply = nullptr;
    QModbusReply* m_currentWriteReply = nullptr;
    QModbusReply* m_multipleCoilsReply = nullptr;
    QModbusReply* m_multipleCoilsWriteReply = nullptr;

    QStandardItemModel* m_coilsReadModel = nullptr;
    QStandardItemModel* m_coilsWriteModel = nullptr;
};