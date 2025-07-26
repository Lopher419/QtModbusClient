#pragma once
#include <QWidget>
#include <QVector>

namespace Ui {
    class CoilTabWidget;
}

class CoilTabWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CoilTabWidget(QWidget* parent = nullptr);
    ~CoilTabWidget();

private slots:
    // 单线圈
    void on_btnWriteSingleCoil_clicked();
    void on_btnReadSingleCoil_clicked();
    // 多线圈
    void on_btnWriteMultipleCoils_clicked();
    void on_btnReadMultipleCoils_clicked();

private:
    Ui::CoilTabWidget* ui;

    // 伪实现，实际请对接你的Modbus通讯逻辑
    bool modbusWriteSingleCoil(int address, bool value);
    bool modbusReadSingleCoil(int address, bool& value);
    bool modbusWriteMultipleCoils(int startAddr, const QVector<bool>& values);
    bool modbusReadMultipleCoils(int startAddr, int count, QVector<bool>& values);

    // 工具函数
    void updateWriteTableRows(int count);
};