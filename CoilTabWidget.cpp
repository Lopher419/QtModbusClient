#include "coiltabwidget.h"
#include "ui_coiltabwidget.h"
#include <QMessageBox>
#include <QTableWidgetItem>

CoilTabWidget::CoilTabWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CoilTabWidget)
{
    ui->setupUi(this);

    // 初始化表格
    int defaultCount = ui->spinBoxWriteCoilCount->value();
    updateWriteTableRows(defaultCount);
    ui->tableWriteCoilStates->setHorizontalHeaderLabels({ "地址", "状态" });
    ui->tableReadCoilStates->setHorizontalHeaderLabels({ "地址", "状态" });

    // 写单线圈 combobox
    ui->comboboxWriteCoilState_2->addItems({ "ON", "OFF" });

    // 信号槽
    connect(ui->btnWriteSingleCoil_2, &QPushButton::clicked, this, &CoilTabWidget::on_btnWriteSingleCoil_2_clicked);
    connect(ui->btnReadSingleCoil, &QPushButton::clicked, this, &CoilTabWidget::on_btnReadSingleCoil_clicked);
    connect(ui->btnWriteMultipleCoils, &QPushButton::clicked, this, &CoilTabWidget::on_btnWriteMultipleCoils_clicked);
    connect(ui->btnReadMultipleCoils, &QPushButton::clicked, this, &CoilTabWidget::on_btnReadMultipleCoils_clicked);

    // 数量变化时调整表格
    connect(ui->spinBoxWriteCoilCount, QOverload<int>::of(&QSpinBox::valueChanged),
        this, &CoilTabWidget::updateWriteTableRows);

    // 初始化表格内容
    updateWriteTableRows(ui->spinBoxWriteCoilCount->value());
}

CoilTabWidget::~CoilTabWidget()
{
    delete ui;
}

// 单线圈写
void CoilTabWidget::on_btnWriteSingleCoil_2_clicked()
{
    int address = ui->spinboxWriteCoilAddress_2->value();
    bool state = (ui->comboboxWriteCoilState_2->currentText() == "ON");
    if (!modbusWriteSingleCoil(address, state))
        QMessageBox::warning(this, "写单线圈", "写入失败！");
}

// 单线圈读
void CoilTabWidget::on_btnReadSingleCoil_clicked()
{
    int address = ui->spinboxReadCoilAddress->value();
    bool state;
    if (modbusReadSingleCoil(address, state))
        ui->labelReadSinpleCoilState->setText(state ? "ON" : "OFF");
    else
        ui->labelReadSinpleCoilState->setText("ERROR");
}

// 多线圈写
void CoilTabWidget::on_btnWriteMultipleCoils_clicked()
{
    int startAddr = ui->spinBoxWriteCoilStartAddress->value();
    int count = ui->spinBoxWriteCoilCount->value();
    QVector<bool> values;
    for (int i = 0; i < count; ++i) {
        auto item = ui->tableWriteCoilStates->item(i, 1);
        values.append(item && item->text() == "ON");
    }
    if (!modbusWriteMultipleCoils(startAddr, values))
        QMessageBox::warning(this, "写多个线圈", "写入失败！");
}

// 多线圈读
void CoilTabWidget::on_btnReadMultipleCoils_clicked()
{
    int startAddr = ui->spinBoxReadCoilStartAddress->value();
    int count = ui->spinBoxReadCoilCount->value();
    QVector<bool> states;
    if (modbusReadMultipleCoils(startAddr, count, states)) {
        ui->tableReadCoilStates->setRowCount(count);
        for (int i = 0; i < count; ++i) {
            ui->tableReadCoilStates->setItem(i, 0, new QTableWidgetItem(QString::number(startAddr + i)));
            ui->tableReadCoilStates->setItem(i, 1, new QTableWidgetItem(states[i] ? "ON" : "OFF"));
        }
    }
    else {
        QMessageBox::warning(this, "读多个线圈", "读取失败！");
    }
}

// 工具：调整写多个线圈表格的行数
void CoilTabWidget::updateWriteTableRows(int count)
{
    ui->tableWriteCoilStates->setRowCount(count);
    for (int i = 0; i < count; ++i) {
        ui->tableWriteCoilStates->setItem(i, 0, new QTableWidgetItem(QString::number(ui->spinBoxWriteCoilStartAddress->value() + i)));
        ui->tableWriteCoilStates->setItem(i, 1, new QTableWidgetItem("OFF"));
    }
}

// 以下为伪实现，实际对接Modbus通讯
bool CoilTabWidget::modbusWriteSingleCoil(int address, bool value)
{
    // TODO: 替换为你的实际通讯代码
    return true;
}

bool CoilTabWidget::modbusReadSingleCoil(int address, bool& value)
{
    // TODO: 替换为你的实际通讯代码
    value = true; // 示例
    return true;
}

bool CoilTabWidget::modbusWriteMultipleCoils(int startAddr, const QVector<bool>& values)
{
    // TODO: 替换为你的实际通讯代码
    return true;
}

bool CoilTabWidget::modbusReadMultipleCoils(int startAddr, int count, QVector<bool>& values)
{
    // TODO: 替换为你的实际通讯代码
    values = QVector<bool>(count, true); // 示例
    return true;
}