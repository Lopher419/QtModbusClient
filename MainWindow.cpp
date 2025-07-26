#include "MainWindow.h"
#include "CoilTabWidget.h"

MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent)
    , m_modbusManager(new ModbusManager(this))
{
    ui.setupUi(this);

    CoilTabWidget* coilTab = new CoilTabWidget(m_modbusManager);

    QWidget* tabCoWidget = ui.tabCO;
    QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(tabCoWidget->layout());
    if (layout) {
        QLayoutItem* oldItem;
        while ((oldItem = layout->takeAt(0)) != nullptr) {
            if (oldItem->widget()) oldItem->widget()->deleteLater();
            delete oldItem;
        }
        layout->addWidget(coilTab);
    }
}

MainWindow::~MainWindow()
{
}