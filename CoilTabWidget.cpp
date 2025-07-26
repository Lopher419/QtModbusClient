#include "CoilTabWidget.h"
#include "ui_CoilTabWidget.h"
#include <QMessageBox>
#include <QTableWidgetItem>
#include "modbusmanager.h"

CoilTabWidget::CoilTabWidget(ModbusManager* modbusManager, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CoilTabWidget)
    , m_modbusManager(modbusManager)  // 注意：这里参数名用对了
{
    ui->setupUi(this);

    int defaultCount = ui->spinBoxWriteCoilCount->value();
    updateWriteTableRows(defaultCount);
    ui->tableWriteCoilStates->setColumnCount(2);
    ui->tableWriteCoilStates->setHorizontalHeaderLabels({ "地址", "状态" });
    ui->tableReadCoilStates->setColumnCount(2);
    ui->tableReadCoilStates->setHorizontalHeaderLabels({ "地址", "状态" });

    ui->comboboxWriteCoilState->addItems({ "ON", "OFF" });

    connect(ui->btnWriteSingleCoil, &QPushButton::clicked, this, &CoilTabWidget::on_btnWriteSingleCoil_clicked);
    connect(ui->btnReadSingleCoil, &QPushButton::clicked, this, &CoilTabWidget::on_btnReadSingleCoil_clicked);
    connect(ui->btnWriteMultipleCoils, &QPushButton::clicked, this, &CoilTabWidget::on_btnWriteMultipleCoils_clicked);
    connect(ui->btnReadMultipleCoils, &QPushButton::clicked, this, &CoilTabWidget::on_btnReadMultipleCoils_clicked);

    connect(ui->spinBoxWriteCoilCount, QOverload<int>::of(&QSpinBox::valueChanged),
        this, &CoilTabWidget::updateWriteTableRows);

    updateWriteTableRows(ui->spinBoxWriteCoilCount->value());
}

CoilTabWidget::~CoilTabWidget()
{
    delete ui;
}

void CoilTabWidget::on_btnWriteSingleCoil_clicked()
{
    int address = ui->spinboxWriteCoilAddress->value();
    bool state = (ui->comboboxWriteCoilState->currentText() == "ON");
    if (!modbusWriteSingleCoil(address, state))
        QMessageBox::warning(this, "写单线圈", "写入失败！");
}

void CoilTabWidget::on_btnReadSingleCoil_clicked()
{
    int address = ui->spinboxReadCoilAddress->value();
    bool state;
    if (modbusReadSingleCoil(address, state))
        ui->labelReadSinpleCoilState->setText(state ? "ON" : "OFF");
    else
        ui->labelReadSinpleCoilState->setText("ERROR");
}

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
        QMessageBox::warning(this, "写多线圈", "写入失败！");
}

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
        QMessageBox::warning(this, "读多线圈", "读取失败！");
    }
}

void CoilTabWidget::updateWriteTableRows(int count)
{
    ui->tableWriteCoilStates->setRowCount(count);
    for (int i = 0; i < count; ++i) {
        ui->tableWriteCoilStates->setItem(i, 0, new QTableWidgetItem(QString::number(ui->spinBoxWriteCoilStartAddress->value() + i)));
        ui->tableWriteCoilStates->setItem(i, 1, new QTableWidgetItem("OFF"));
    }
}

bool CoilTabWidget::modbusWriteSingleCoil(int address, bool value)
{
    if (!m_modbusManager || !m_modbusManager->isConnected()) {
        qWarning() << "Modbus not connected";
        return false;
    }

    QModbusDataUnit writeUnit(QModbusDataUnit::Coils, address, 1);
    writeUnit.setValue(0, value ? 0xFF00 : 0x0000);

    if (QModbusReply* reply = m_modbusManager->master()->sendWriteRequest(
        writeUnit, 1))
    {
        if (!reply->isFinished()) {
            QEventLoop loop;
            connect(reply, &QModbusReply::finished, &loop, &QEventLoop::quit);
            loop.exec();
        }

        const bool success = (reply->error() == QModbusDevice::NoError);
        reply->deleteLater();
        return success;
    }
    return false;
}

bool CoilTabWidget::modbusReadSingleCoil(int address, bool& value)
{
    if (!m_modbusManager || !m_modbusManager->isConnected()) {
        qWarning() << "Modbus not connected";
        return false;
    }

    QModbusDataUnit request(QModbusDataUnit::Coils, address, 1);

    if (QModbusReply* reply = m_modbusManager->master()->sendReadRequest(
        request, 1))
    {
        if (!reply->isFinished()) {
            QEventLoop loop;
            connect(reply, &QModbusReply::finished, &loop, &QEventLoop::quit);
            loop.exec();
        }

        if (reply->error() == QModbusDevice::NoError) {
            const QModbusDataUnit result = reply->result();
            value = (result.value(0) & 0x01);
            reply->deleteLater();
            return true;
        }
        reply->deleteLater();
    }
    return false;
}

bool CoilTabWidget::modbusWriteMultipleCoils(int startAddr, const QVector<bool>& values)
{
    if (!m_modbusManager || !m_modbusManager->isConnected()) {
        qWarning() << "Modbus not connected";
        return false;
    }

    QModbusDataUnit writeUnit(QModbusDataUnit::Coils, startAddr, values.size());

    for (int i = 0; i < values.size(); ++i) {
        writeUnit.setValue(i, values[i] ? 0xFF00 : 0x0000);
    }

    if (QModbusReply* reply = m_modbusManager->master()->sendWriteRequest(
        writeUnit, 1))
    {
        if (!reply->isFinished()) {
            QEventLoop loop;
            connect(reply, &QModbusReply::finished, &loop, &QEventLoop::quit);
            loop.exec();
        }

        const bool success = (reply->error() == QModbusDevice::NoError);
        reply->deleteLater();
        return success;
    }
    return false;
}

bool CoilTabWidget::modbusReadMultipleCoils(int startAddr, int count, QVector<bool>& values)
{
    if (!m_modbusManager || !m_modbusManager->isConnected()) {
        qWarning() << "Modbus not connected";
        return false;
    }

    QModbusDataUnit request(QModbusDataUnit::Coils, startAddr, count);

    if (QModbusReply* reply = m_modbusManager->master()->sendReadRequest(
        request, 1))
    {
        if (!reply->isFinished()) {
            QEventLoop loop;
            connect(reply, &QModbusReply::finished, &loop, &QEventLoop::quit);
            loop.exec();
        }

        if (reply->error() == QModbusDevice::NoError) {
            const QModbusDataUnit result = reply->result();
            values.resize(result.valueCount());
            for (int i = 0; i < result.valueCount(); ++i) {
                values[i] = (result.value(i) & 0x01);
            }
            reply->deleteLater();
            return true;
        }
        reply->deleteLater();
    }
    return false;
}