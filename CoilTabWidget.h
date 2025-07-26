#pragma once
#include <QWidget>
#include <QVector>
#include "modbusmanager.h"

namespace Ui {
    class CoilTabWidget;
}

class CoilTabWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CoilTabWidget(ModbusManager* modbusManager, QWidget* parent = nullptr);
    ~CoilTabWidget();

private slots:
    void on_btnWriteSingleCoil_clicked();
    void on_btnReadSingleCoil_clicked();
    void on_btnWriteMultipleCoils_clicked();
    void on_btnReadMultipleCoils_clicked();

private:
    Ui::CoilTabWidget* ui;
    ModbusManager* m_modbusManager;

    bool modbusWriteSingleCoil(int address, bool value);
    bool modbusReadSingleCoil(int address, bool& value);
    bool modbusWriteMultipleCoils(int startAddr, const QVector<bool>& values);
    bool modbusReadMultipleCoils(int startAddr, int count, QVector<bool>& values);

    void updateWriteTableRows(int count);
};