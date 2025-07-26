#include "coiltabwidget.h"
#include "ui_coiltabwidget.h"
#include <QMessageBox>
#include <QTableWidgetItem>

CoilTabWidget::CoilTabWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CoilTabWidget)
{
    ui->setupUi(this);

    // ��ʼ�����
    int defaultCount = ui->spinBoxWriteCoilCount->value();
    updateWriteTableRows(defaultCount);
    ui->tableWriteCoilStates->setHorizontalHeaderLabels({ "��ַ", "״̬" });
    ui->tableReadCoilStates->setHorizontalHeaderLabels({ "��ַ", "״̬" });

    // д����Ȧ combobox
    ui->comboboxWriteCoilState_2->addItems({ "ON", "OFF" });

    // �źŲ�
    connect(ui->btnWriteSingleCoil_2, &QPushButton::clicked, this, &CoilTabWidget::on_btnWriteSingleCoil_2_clicked);
    connect(ui->btnReadSingleCoil, &QPushButton::clicked, this, &CoilTabWidget::on_btnReadSingleCoil_clicked);
    connect(ui->btnWriteMultipleCoils, &QPushButton::clicked, this, &CoilTabWidget::on_btnWriteMultipleCoils_clicked);
    connect(ui->btnReadMultipleCoils, &QPushButton::clicked, this, &CoilTabWidget::on_btnReadMultipleCoils_clicked);

    // �����仯ʱ�������
    connect(ui->spinBoxWriteCoilCount, QOverload<int>::of(&QSpinBox::valueChanged),
        this, &CoilTabWidget::updateWriteTableRows);

    // ��ʼ���������
    updateWriteTableRows(ui->spinBoxWriteCoilCount->value());
}

CoilTabWidget::~CoilTabWidget()
{
    delete ui;
}

// ����Ȧд
void CoilTabWidget::on_btnWriteSingleCoil_2_clicked()
{
    int address = ui->spinboxWriteCoilAddress_2->value();
    bool state = (ui->comboboxWriteCoilState_2->currentText() == "ON");
    if (!modbusWriteSingleCoil(address, state))
        QMessageBox::warning(this, "д����Ȧ", "д��ʧ�ܣ�");
}

// ����Ȧ��
void CoilTabWidget::on_btnReadSingleCoil_clicked()
{
    int address = ui->spinboxReadCoilAddress->value();
    bool state;
    if (modbusReadSingleCoil(address, state))
        ui->labelReadSinpleCoilState->setText(state ? "ON" : "OFF");
    else
        ui->labelReadSinpleCoilState->setText("ERROR");
}

// ����Ȧд
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
        QMessageBox::warning(this, "д�����Ȧ", "д��ʧ�ܣ�");
}

// ����Ȧ��
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
        QMessageBox::warning(this, "�������Ȧ", "��ȡʧ�ܣ�");
    }
}

// ���ߣ�����д�����Ȧ��������
void CoilTabWidget::updateWriteTableRows(int count)
{
    ui->tableWriteCoilStates->setRowCount(count);
    for (int i = 0; i < count; ++i) {
        ui->tableWriteCoilStates->setItem(i, 0, new QTableWidgetItem(QString::number(ui->spinBoxWriteCoilStartAddress->value() + i)));
        ui->tableWriteCoilStates->setItem(i, 1, new QTableWidgetItem("OFF"));
    }
}

// ����Ϊαʵ�֣�ʵ�ʶԽ�ModbusͨѶ
bool CoilTabWidget::modbusWriteSingleCoil(int address, bool value)
{
    // TODO: �滻Ϊ���ʵ��ͨѶ����
    return true;
}

bool CoilTabWidget::modbusReadSingleCoil(int address, bool& value)
{
    // TODO: �滻Ϊ���ʵ��ͨѶ����
    value = true; // ʾ��
    return true;
}

bool CoilTabWidget::modbusWriteMultipleCoils(int startAddr, const QVector<bool>& values)
{
    // TODO: �滻Ϊ���ʵ��ͨѶ����
    return true;
}

bool CoilTabWidget::modbusReadMultipleCoils(int startAddr, int count, QVector<bool>& values)
{
    // TODO: �滻Ϊ���ʵ��ͨѶ����
    values = QVector<bool>(count, true); // ʾ��
    return true;
}