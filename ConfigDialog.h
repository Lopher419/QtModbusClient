#pragma once

#include <QDialog>
#include <QSerialPort>
#include "ui_ConfigDialog.h"

namespace Ui {
	class ConfigDialog;
}

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget* parent = nullptr);
    ~ConfigDialog();

    QString portName() const;
    QSerialPort::BaudRate baudRate() const;
    QSerialPort::DataBits dataBits() const;
    QSerialPort::StopBits stopBits() const;
    QSerialPort::Parity parity() const;
    QSerialPort::FlowControl flowControl() const;

private slots:
    void populatePorts();
    void populateBaudRates();

private:
    Ui::ConfigDialog* ui;
};
