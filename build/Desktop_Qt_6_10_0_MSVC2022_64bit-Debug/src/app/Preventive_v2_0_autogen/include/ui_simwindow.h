/********************************************************************************
** Form generated from reading UI file 'simwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.10.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SIMWINDOW_H
#define UI_SIMWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QPushButton>
#include "qcustomplot.h"

QT_BEGIN_NAMESPACE

class Ui_SimWindow
{
public:
    QPushButton *closeSimulatorBtn;
    QCustomPlot *plotWidget;

    void setupUi(QDialog *SimWindow)
    {
        if (SimWindow->objectName().isEmpty())
            SimWindow->setObjectName("SimWindow");
        SimWindow->resize(800, 600);
        closeSimulatorBtn = new QPushButton(SimWindow);
        closeSimulatorBtn->setObjectName("closeSimulatorBtn");
        closeSimulatorBtn->setGeometry(QRect(680, 530, 80, 24));
        plotWidget = new QCustomPlot(SimWindow);
        plotWidget->setObjectName("plotWidget");
        plotWidget->setGeometry(QRect(39, 29, 321, 191));

        retranslateUi(SimWindow);

        QMetaObject::connectSlotsByName(SimWindow);
    } // setupUi

    void retranslateUi(QDialog *SimWindow)
    {
        SimWindow->setWindowTitle(QCoreApplication::translate("SimWindow", "Dialog", nullptr));
        closeSimulatorBtn->setText(QCoreApplication::translate("SimWindow", "Zamknij", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SimWindow: public Ui_SimWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SIMWINDOW_H
