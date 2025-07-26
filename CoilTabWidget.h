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
    // ����Ȧ
    void on_btnWriteSingleCoil_clicked();
    void on_btnReadSingleCoil_clicked();
    // ����Ȧ
    void on_btnWriteMultipleCoils_clicked();
    void on_btnReadMultipleCoils_clicked();

private:
    Ui::CoilTabWidget* ui;

    // αʵ�֣�ʵ����Խ����ModbusͨѶ�߼�
    bool modbusWriteSingleCoil(int address, bool value);
    bool modbusReadSingleCoil(int address, bool& value);
    bool modbusWriteMultipleCoils(int startAddr, const QVector<bool>& values);
    bool modbusReadMultipleCoils(int startAddr, int count, QVector<bool>& values);

    // ���ߺ���
    void updateWriteTableRows(int count);
};