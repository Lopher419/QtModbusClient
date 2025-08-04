#include "CoilWidget.h"
#include <QMessageBox>
#include <QPoint>
#include <QDebug>
#include <QEventLoop>
#include <QModbusReply>
#include <QModbusDataUnit>

CoilWidget::CoilWidget(QWidget *parent)
	: QWidget(parent)
{
    ui.setupUi(this);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    initSingleCoilUI();
    initMultipleCoilsReadUI();
    initMultipleCoilsWriteUI();
    initTableModels();

    connect(ui.coilReadBtn, &QPushButton::clicked, this, &CoilWidget::onReadCoil);
    connect(ui.coilWriteBtn, &QPushButton::clicked, this, &CoilWidget::onWriteCoil);
    connect(ui.coilsReadBtn, &QPushButton::clicked, this, &CoilWidget::onReadMultipleCoils);
    connect(ui.coilsWriteBtn, &QPushButton::clicked, this, &CoilWidget::onWriteMultipleCoils);
    connect(ui.coilsWriteSelectAllCheckBox, &QCheckBox::stateChanged, this, &CoilWidget::onSelectAllChanged);
    connect(ui.coilsWriteAddressSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
        this, [this](int) { updateCoilsWriteTable(); });
    connect(ui.coilsWriteCountSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
        this, [this](int) { updateCoilsWriteTable(); });
    connect(m_coilsWriteModel, &QStandardItemModel::itemChanged,
        this, &CoilWidget::handleCoilWriteItemChanged);

    updateCoilsWriteTable();
}

CoilWidget::~CoilWidget()
{
    auto safeDeleteReply = [](QModbusReply* reply) {
        if (reply) {
            disconnect(reply, nullptr, nullptr, nullptr);
            if (!reply->isFinished()) {
                QEventLoop loop;
                connect(reply, &QModbusReply::finished, &loop, &QEventLoop::quit);
                loop.exec(QEventLoop::ExcludeUserInputEvents);
            }
            reply->deleteLater();
        }
        };

    safeDeleteReply(m_currentReply);
    safeDeleteReply(m_currentWriteReply);
    safeDeleteReply(m_multipleCoilsReply);
    safeDeleteReply(m_multipleCoilsWriteReply);
}

void CoilWidget::setModbusConnection(ModbusConnection* connection)
{
	m_modbusConnection = connection;
}

// Handles single coil read request
void CoilWidget::onReadCoil()
{
    if (!m_modbusConnection || !m_modbusConnection->isConnected()) {
        QMessageBox::warning(this, "Error", "Not connected to any device");
        return;
    }

    int address = ui.coilReadAddressSpinBox->value();

    if (m_currentReply) {
        disconnect(m_currentReply, nullptr, this, nullptr);
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }

    ui.coilReadBtn->setEnabled(false);
    m_currentReply = m_modbusConnection->readRegister(ModbusConnection::Coils, address, 1);

    if (m_currentReply) {
        connect(m_currentReply, &QModbusReply::finished, this, &CoilWidget::handleCoilReadResult);
    }
    else {
        QMessageBox::critical(this, "Error", "Failed to send read request");
        ui.coilReadBtn->setEnabled(true);
    }
}

// Processes result from single coil read
void CoilWidget::handleCoilReadResult()
{
    ui.coilReadBtn->setEnabled(true);

    auto* reply = qobject_cast<QModbusReply*>(sender());
    if (!reply || reply != m_currentReply) {
        if (reply) reply->deleteLater();
        return;
    }

    if (reply->error() != QModbusDevice::NoError) {
        qDebug() << "Coil read error:" << reply->errorString();
        reply->deleteLater();
        m_currentReply = nullptr;
        return;
    }

    const QModbusDataUnit result = reply->result();
    if (result.registerType() == QModbusDataUnit::Coils && result.valueCount() > 0) {
        bool coilState = result.value(0);
        ui.coilReadDataSpinBox->setValue(coilState ? 1 : 0);

        int address = ui.coilReadAddressSpinBox->value();
        qDebug() << "Coil read successful - Address:" << address
            << "Value:" << coilState;
        QApplication::beep();
    }
    else {
        qDebug() << "Invalid coil data received";
    }

    reply->deleteLater();
    m_currentReply = nullptr;
}

// Handles single coil write request
void CoilWidget::onWriteCoil()
{
    if (!m_modbusConnection || !m_modbusConnection->isConnected()) {
        QMessageBox::warning(this, "Error", "Not connected to any device");
        return;
    }

    int address = ui.coilWriteAddressSpinBox->value();
    bool value = ui.coilWriteDataComboBox->currentData().toBool();

    if (m_currentWriteReply) {
        disconnect(m_currentWriteReply, nullptr, this, nullptr);
        m_currentWriteReply->deleteLater();
        m_currentWriteReply = nullptr;
    }

    ui.coilWriteBtn->setEnabled(false);
    m_currentWriteReply = m_modbusConnection->writeCoil(address, value);

    if (!m_currentWriteReply) {
        QMessageBox::critical(this, "Error", "Failed to create write request");
        ui.coilWriteBtn->setEnabled(true);
        return;
    }

    connect(m_currentWriteReply, &QModbusReply::finished, this, &CoilWidget::handleCoilWriteResult);
}

// Processes result from single coil write
void CoilWidget::handleCoilWriteResult()
{
    ui.coilWriteBtn->setEnabled(true);

    auto* reply = qobject_cast<QModbusReply*>(sender());
    if (!reply || reply != m_currentWriteReply) {
        if (reply) reply->deleteLater();
        return;
    }

    int address = ui.coilWriteAddressSpinBox->value();
    bool value = ui.coilWriteDataComboBox->currentData().toBool();

    if (reply->error() == QModbusDevice::NoError) {
        qDebug() << "Coil write successful - Address:" << address << "Value:" << value;
        QApplication::beep();
    } else {
        qDebug() << "Coil write error:" << reply->errorString();
    }

    reply->deleteLater();
    m_currentWriteReply = nullptr;
}

// Handles multi-coil read request
void CoilWidget::onReadMultipleCoils()
{
    if (!m_modbusConnection || !m_modbusConnection->isConnected()) {
        QMessageBox::warning(this, tr("Error"), tr("Not connected to any device"));
        return;
    }

    int address = ui.coilsReadAddressSpinBox->value();
    int count = ui.coilsReadCountSpinBox->value();

    if (m_multipleCoilsReply) {
        disconnect(m_multipleCoilsReply, nullptr, this, nullptr);
        m_multipleCoilsReply->deleteLater();
        m_multipleCoilsReply = nullptr;
    }

    ui.coilsReadBtn->setEnabled(false);
    m_multipleCoilsReply = m_modbusConnection->readRegister(
        ModbusConnection::Coils, address, count);

    if (m_multipleCoilsReply) {
        connect(m_multipleCoilsReply, &QModbusReply::finished,
            this, &CoilWidget::handleMultipleCoilsReadResult);
    }
    else {
        QMessageBox::critical(this, tr("Error"), tr("Failed to send read request"));
        ui.coilsReadBtn->setEnabled(true);
    }
}

// Processes multi-coil read results into table
void CoilWidget::handleMultipleCoilsReadResult()
{
    ui.coilsReadBtn->setEnabled(true);

    auto* reply = qobject_cast<QModbusReply*>(sender());
    if (!reply || reply != m_multipleCoilsReply) {
        if (reply) reply->deleteLater();
        return;
    }

    if (reply->error() != QModbusDevice::NoError) {
        qDebug() << "Multiple coils read error:" << reply->errorString();
        reply->deleteLater();
        m_multipleCoilsReply = nullptr;
        return;
    }

    const QModbusDataUnit result = reply->result();
    if (result.registerType() == QModbusDataUnit::Coils && result.valueCount() > 0) {
        m_coilsReadModel->removeRows(0, m_coilsReadModel->rowCount());
        int startAddr = result.startAddress();
        int byteCount = (result.valueCount() + 7) / 8;
        QVector<quint8> bytes(byteCount, 0);

        for (int i = 0; i < result.valueCount(); ++i) {
            bool value = result.value(i);
            int addr = startAddr + i;

            int byteIndex = i / 8;
            int bitIndex = i % 8;
            if (value) {
                bytes[byteIndex] |= (1 << bitIndex);
            }

            QList<QStandardItem*> rowItems;
            rowItems << new QStandardItem(QString::number(addr));
            rowItems << new QStandardItem();
            rowItems << new QStandardItem(value ? "1" : "0");
            m_coilsReadModel->appendRow(rowItems);
        }

        for (int i = 0; i < byteCount; ++i) {
            int rowIndex = i * 8;
            if (rowIndex < m_coilsReadModel->rowCount()) {
                QModelIndex index = m_coilsReadModel->index(rowIndex, 1);
                m_coilsReadModel->setData(index, QString("0x%1").arg(bytes[i], 2, 16, QChar('0')).toUpper());
            }
        }

        qDebug() << "Multiple coils read successful - Start address:"
            << startAddr << "Count:" << result.valueCount();
        QApplication::beep();
    }
    else {
        qDebug() << "Invalid multiple coils data received";
    }

    reply->deleteLater();
    m_multipleCoilsReply = nullptr;
}

// Handles multi-coil write request
void CoilWidget::onWriteMultipleCoils()
{
    if (!m_modbusConnection || !m_modbusConnection->isConnected()) {
        QMessageBox::warning(this, tr("Error"), tr("Not connected to any device"));
        return;
    }

    int startAddr = ui.coilsWriteAddressSpinBox->value();
    int count = ui.coilsWriteCountSpinBox->value();

    QVector<quint16> values;
    for (int i = 0; i < count; ++i) {
        QModelIndex index = m_coilsWriteModel->index(i, 1);
        bool isChecked = (m_coilsWriteModel->data(index, Qt::CheckStateRole).toInt() == Qt::Checked);
        values.append(isChecked ? 1 : 0);
    }

    if (m_multipleCoilsWriteReply) {
        disconnect(m_multipleCoilsWriteReply, nullptr, this, nullptr);
        m_multipleCoilsWriteReply->deleteLater();
        m_multipleCoilsWriteReply = nullptr;
    }

    ui.coilsWriteBtn->setEnabled(false);
    m_multipleCoilsWriteReply = m_modbusConnection->writeMultipleRegisters(
        ModbusConnection::Coils, startAddr, values);

    if (m_multipleCoilsWriteReply) {
        connect(m_multipleCoilsWriteReply, &QModbusReply::finished,
            this, &CoilWidget::handleMultipleCoilsWriteResult);
    }
    else {
        QMessageBox::critical(this, tr("Error"), tr("Failed to send write request"));
        ui.coilsWriteBtn->setEnabled(true);
    }
}

// Processes multi-coil write result
void CoilWidget::handleMultipleCoilsWriteResult()
{
    ui.coilsWriteBtn->setEnabled(true);

    auto* reply = qobject_cast<QModbusReply*>(sender());
    if (!reply || reply != m_multipleCoilsWriteReply) {
        if (reply) reply->deleteLater();
        return;
    }

    int startAddr = ui.coilsWriteAddressSpinBox->value();
    int count = ui.coilsWriteCountSpinBox->value();

    if (reply->error() == QModbusDevice::NoError) {
        qDebug() << "Multiple coils write successful - Start address:"
            << startAddr << "Count:" << count;
        QApplication::beep();
        onReadMultipleCoils();
    }
    else {
        qDebug() << "Multiple coils write error:" << reply->errorString();
        QMessageBox::critical(this, tr("Error"),
            tr("Write failed: %1").arg(reply->errorString()));
    }

    reply->deleteLater();
    m_multipleCoilsWriteReply = nullptr;
}

// Toggles select-all state for write table
void CoilWidget::onSelectAllChanged(int state)
{
    bool selectAll = (state == Qt::Checked);

    for (int row = 0; row < m_coilsWriteModel->rowCount(); ++row) {
        QModelIndex index = m_coilsWriteModel->index(row, 1);
        m_coilsWriteModel->setData(index, selectAll ? Qt::Checked : Qt::Unchecked,
            Qt::CheckStateRole);
    }
}

void CoilWidget::initSingleCoilUI()
{
    ui.coilReadAddressSpinBox->setRange(0, 65535);
    ui.coilReadDataSpinBox->setReadOnly(true);

    ui.coilWriteAddressSpinBox->setRange(0, 65535);
    ui.coilWriteDataComboBox->addItem("0", false);
    ui.coilWriteDataComboBox->addItem("1", true);
}

void CoilWidget::initMultipleCoilsReadUI()
{
    ui.coilsReadAddressSpinBox->setRange(0, 65535);
    ui.coilsReadCountSpinBox->setRange(1, 2000);
    ui.coilsReadCountSpinBox->setValue(16);
}

void CoilWidget::initMultipleCoilsWriteUI()
{
    ui.coilsWriteAddressSpinBox->setRange(0, 65535);
    ui.coilsWriteCountSpinBox->setRange(1, 2000);
    ui.coilsWriteCountSpinBox->setValue(16);
    ui.coilsWriteTableView->setEditTriggers(QAbstractItemView::AllEditTriggers);
    ui.coilsWriteTableView->setSelectionBehavior(QAbstractItemView::SelectItems);
}

void CoilWidget::initTableModels()
{
    m_coilsReadModel = new QStandardItemModel(this);
    m_coilsReadModel->setColumnCount(3);
    m_coilsReadModel->setHeaderData(0, Qt::Horizontal, tr("Address"));
    m_coilsReadModel->setHeaderData(1, Qt::Horizontal, tr("Hex"));
    m_coilsReadModel->setHeaderData(2, Qt::Horizontal, tr("Binary"));
    ui.coilsReadTableView->setModel(m_coilsReadModel);
    ui.coilsReadTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    m_coilsWriteModel = new QStandardItemModel(this);
    m_coilsWriteModel->setColumnCount(3);
    m_coilsWriteModel->setHeaderData(0, Qt::Horizontal, tr("Address"));
    m_coilsWriteModel->setHeaderData(1, Qt::Horizontal, tr("Hex"));
    m_coilsWriteModel->setHeaderData(2, Qt::Horizontal, tr("Binary"));
    ui.coilsWriteTableView->setModel(m_coilsWriteModel);
    ui.coilsWriteTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

// Refreshes write table based on address/count inputs
void CoilWidget::updateCoilsWriteTable()
{
    int startAddr = ui.coilsWriteAddressSpinBox->value();
    int count = ui.coilsWriteCountSpinBox->value();

    m_coilsWriteModel->removeRows(0, m_coilsWriteModel->rowCount());

    for (int i = 0; i < count; ++i) {
        QList<QStandardItem*> rowItems;
        int address = startAddr + i;

        QStandardItem* addrItem = new QStandardItem(QString::number(address));
        addrItem->setEditable(false);

        QStandardItem* valueItem = new QStandardItem();
        valueItem->setCheckable(true);
        valueItem->setCheckState(Qt::Unchecked);
        valueItem->setData(address, Qt::UserRole);

        QStandardItem* binItem = new QStandardItem("0");
        binItem->setEditable(false);

        rowItems << addrItem << valueItem << binItem;
        m_coilsWriteModel->appendRow(rowItems);
    }
}

// Updates binary display when write table checkbox changes
void CoilWidget::handleCoilWriteItemChanged(QStandardItem* item)
{
    if (item->column() == 1) {
        int row = item->row();
        QStandardItem* binItem = m_coilsWriteModel->item(row, 2);
        if (binItem) {
            binItem->setText(item->checkState() == Qt::Checked ? "1" : "0");
        }
    }
}