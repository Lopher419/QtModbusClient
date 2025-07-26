#include "MainWindow.h"
#include "CoilTabWidget.h"

MainWindow::MainWindow(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

    QWidget* tabCoWidget = ui.tabCO;
    QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(tabCoWidget->layout());
    if (layout) {
        QLayoutItem* oldItem;
        while ((oldItem = layout->takeAt(0)) != nullptr) {
            if (oldItem->widget()) oldItem->widget()->deleteLater();
            delete oldItem;
        }
        CoilTabWidget* coilTab = new CoilTabWidget;
        layout->addWidget(coilTab);
    }
}

MainWindow::~MainWindow()
{

}
