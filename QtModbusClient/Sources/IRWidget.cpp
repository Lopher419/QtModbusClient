#include "IRWidget.h"
#include <QMessageBox>
#include <QDebug>
#include <QEventLoop>
#include <QHeaderView>
#include <QApplication>
#include <QStandardItem>
#include <QIntValidator>

IRWidget::IRWidget(QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    initUI();
    initTableModel();
    setupConnections();
}

IRWidget::~IRWidget()
{
    safeDeleteReply(m_singleIRReadReply);
    safeDeleteReply(m_multipleIRReadReply);
}

void IRWidget::setModbusConnection(ModbusConnection* connection)
{
    m_modbusConnection = connection;
}

void IRWidget::setupConnections()
{
    connect(ui.irReadSingleBtn, &QPushButton::clicked, this, &IRWidget::onReadSingleIR);
    connect(ui.irReadMultipleBtn, &QPushButton::clicked, this, &IRWidget::onReadMultipleIR);
}

void IRWidget::initUI()
{
    ui.irReadSingleAddressSpinBox->setRange(0, 65535);
    ui.irReadSingleDataLineEdit->setReadOnly(true);

    ui.irReadMultipleAddressSpinBox->setRange(0, 65535);
    ui.irReadMultipleCountSpinBox->setRange(1, 125);
    ui.irReadMultipleCountSpinBox->setValue(10);
}

void IRWidget::initTableModel()
{
    m_irModel = new QStandardItemModel(this);
    m_irModel->setColumnCount(3);
    m_irModel->setHorizontalHeaderLabels({ tr("Address"), tr("Hex"), tr("Decimal") });
    ui.irReadMultipleDataTableView->setModel(m_irModel);
    ui.irReadMultipleDataTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui.irReadMultipleDataTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void IRWidget::safeDeleteReply(QModbusReply* reply)
{
    if (!reply) return;

    disconnect(reply, nullptr, nullptr, nullptr);
    if (!reply->isFinished()) {
        QEventLoop loop;
        connect(reply, &QModbusReply::finished, &loop, &QEventLoop::quit);
        loop.exec(QEventLoop::ExcludeUserInputEvents);
    }
    reply->deleteLater();
}

// Handle single input register read request
void IRWidget::onReadSingleIR()
{
    if (!m_modbusConnection || !m_modbusConnection->isConnected()) {
        QMessageBox::warning(this, "Error", "Not connected to any device");
        return;
    }

    safeDeleteReply(m_singleIRReadReply);
    ui.irReadSingleBtn->setEnabled(false);

    const int address = ui.irReadSingleAddressSpinBox->value();
    m_singleIRReadReply = m_modbusConnection->readRegister(
        ModbusConnection::InputRegisters, address, 1);

    if (m_singleIRReadReply) {
        connect(m_singleIRReadReply, &QModbusReply::finished,
            this, &IRWidget::handleSingleIRReadResult);
    }
    else {
        QMessageBox::critical(this, "Error", "Failed to send read request");
        ui.irReadSingleBtn->setEnabled(true);
    }
}

// Process single input register read result
void IRWidget::handleSingleIRReadResult()
{
    auto* reply = qobject_cast<QModbusReply*>(sender());
    ui.irReadSingleBtn->setEnabled(true);

    if (!reply || reply != m_singleIRReadReply) {
        if (reply) reply->deleteLater();
        m_singleIRReadReply = nullptr;
        return;
    }

    if (reply->error() == QModbusDevice::NoError) {
        processSingleIRResult(reply);
    }
    else {
        qDebug() << "Single IR read error:" << reply->errorString();
        QMessageBox::critical(this, "Error",
            tr("Read failed: %1").arg(reply->errorString()));
    }

    safeDeleteReply(m_singleIRReadReply);
}

void IRWidget::processSingleIRResult(QModbusReply* reply)
{
    const QModbusDataUnit result = reply->result();
    if (result.registerType() == QModbusDataUnit::InputRegisters && result.valueCount() > 0) {
        const quint16 value = result.value(0);
        ui.irReadSingleDataLineEdit->setText(QString::number(value));

        const int address = ui.irReadSingleAddressSpinBox->value();
        qDebug() << "IR read successful - Address:" << address << "Value:" << value;
        QApplication::beep();
    }
    else {
        qDebug() << "Invalid IR data received";
        QMessageBox::warning(this, "Warning", "Invalid data received from device");
    }
}

// Handle multiple input registers read request
void IRWidget::onReadMultipleIR()
{
    if (!m_modbusConnection || !m_modbusConnection->isConnected()) {
        QMessageBox::warning(this, tr("Error"), tr("Not connected to any device"));
        return;
    }

    safeDeleteReply(m_multipleIRReadReply);
    ui.irReadMultipleBtn->setEnabled(false);

    const int address = ui.irReadMultipleAddressSpinBox->value();
    const int count = ui.irReadMultipleCountSpinBox->value();

    m_multipleIRReadReply = m_modbusConnection->readRegister(
        ModbusConnection::InputRegisters, address, count);

    if (m_multipleIRReadReply) {
        connect(m_multipleIRReadReply, &QModbusReply::finished,
            this, &IRWidget::handleMultipleIRReadResult);
    }
    else {
        QMessageBox::critical(this, tr("Error"), tr("Failed to send read request"));
        ui.irReadMultipleBtn->setEnabled(true);
    }
}

// Process multiple input registers read results
void IRWidget::handleMultipleIRReadResult()
{
    auto* reply = qobject_cast<QModbusReply*>(sender());
    ui.irReadMultipleBtn->setEnabled(true);

    if (!reply || reply != m_multipleIRReadReply) {
        if (reply) reply->deleteLater();
        m_multipleIRReadReply = nullptr;
        return;
    }

    if (reply->error() == QModbusDevice::NoError) {
        processMultipleIRResult(reply);
    }
    else {
        qDebug() << "Multiple IR read error:" << reply->errorString();
        QMessageBox::critical(this, "Error",
            tr("Read failed: %1").arg(reply->errorString()));
    }

    safeDeleteReply(m_multipleIRReadReply);
}

void IRWidget::processMultipleIRResult(QModbusReply* reply)
{
    const QModbusDataUnit result = reply->result();
    if (result.registerType() != QModbusDataUnit::InputRegisters || result.valueCount() <= 0) {
        qDebug() << "Invalid multiple IR data received";
        QMessageBox::warning(this, "Warning", "Invalid data received from device");
        return;
    }

    m_irModel->removeRows(0, m_irModel->rowCount());
    const int startAddr = result.startAddress();
    const int count = result.valueCount();

    QVector<quint16> values;
    for (int i = 0; i < count; ++i) {
        values.append(result.value(i));
    }

    updateHexColumn(values, startAddr);
    qDebug() << "Multiple IR read successful - Start address:" << startAddr << "Count:" << count;
    QApplication::beep();
}

void IRWidget::updateHexColumn(const QVector<quint16>& values, int startAddr)
{
    for (int i = 0; i < values.size(); ++i) {
        QList<QStandardItem*> rowItems;
        const int addr = startAddr + i;
        const quint16 value = values[i];

        rowItems << new QStandardItem(QString::number(addr));
        rowItems << new QStandardItem(QString("0x%1").arg(value, 4, 16, QChar('0')).toUpper());
        rowItems << new QStandardItem(QString::number(value));

        for (auto item : rowItems) {
            item->setEditable(false);
        }

        m_irModel->appendRow(rowItems);
    }
}