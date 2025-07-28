#include "ConfigDialog.h"

ConfigDialog::ConfigDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);

    populatePorts();

    populateBaudRates();

    ui->dataBitsComboBox->addItem("5", QSerialPort::Data5);
    ui->dataBitsComboBox->addItem("6", QSerialPort::Data6);
    ui->dataBitsComboBox->addItem("7", QSerialPort::Data7);
    ui->dataBitsComboBox->addItem("8", QSerialPort::Data8);
    ui->dataBitsComboBox->setCurrentIndex(3);

    ui->stopBitsComboBox->addItem("1", QSerialPort::OneStop);
    ui->stopBitsComboBox->addItem("1.5", QSerialPort::OneAndHalfStop);
    ui->stopBitsComboBox->addItem("2", QSerialPort::TwoStop);

    ui->parityComboBox->addItem("None", QSerialPort::NoParity);
    ui->parityComboBox->addItem("Even", QSerialPort::EvenParity);
    ui->parityComboBox->addItem("Odd", QSerialPort::OddParity);
    ui->parityComboBox->addItem("Space", QSerialPort::SpaceParity);
    ui->parityComboBox->addItem("Mark", QSerialPort::MarkParity);

    ui->flowControlComboBox->addItem("None", QSerialPort::NoFlowControl);
    ui->flowControlComboBox->addItem("RTS/CTS", QSerialPort::HardwareControl);
    ui->flowControlComboBox->addItem("XON/XOFF", QSerialPort::SoftwareControl);
}

void ConfigDialog::populatePorts()
{
    ui->portComboBox->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo& port : ports) {
        ui->portComboBox->addItem(port.portName());
    }
}

void ConfigDialog::populateBaudRates()
{
    ui->baudRateComboBox->clear();
    const QList<QSerialPort::BaudRate> baudRates = {
        QSerialPort::Baud1200,
        QSerialPort::Baud2400,
        QSerialPort::Baud4800,
        QSerialPort::Baud9600,
        QSerialPort::Baud19200,
        QSerialPort::Baud38400,
        QSerialPort::Baud57600,
        QSerialPort::Baud115200
    };

    for (const auto baudRate : baudRates) {
        ui->baudRateComboBox->addItem(QString::number(baudRate), baudRate);
    }
    ui->baudRateComboBox->setCurrentText("9600");
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

QString ConfigDialog::portName() const
{
    return ui->portComboBox->currentText();
}

QSerialPort::BaudRate ConfigDialog::baudRate() const
{
    return static_cast<QSerialPort::BaudRate>(
        ui->baudRateComboBox->currentData().toInt());
}

QSerialPort::DataBits ConfigDialog::dataBits() const
{
    return static_cast<QSerialPort::DataBits>(
        ui->dataBitsComboBox->currentData().toInt());
}

QSerialPort::StopBits ConfigDialog::stopBits() const
{
    return static_cast<QSerialPort::StopBits>(
        ui->stopBitsComboBox->currentData().toInt());
}

QSerialPort::Parity ConfigDialog::parity() const
{
    return static_cast<QSerialPort::Parity>(
        ui->parityComboBox->currentData().toInt());
}

QSerialPort::FlowControl ConfigDialog::flowControl() const
{
    return static_cast<QSerialPort::FlowControl>(
        ui->flowControlComboBox->currentData().toInt());
}
