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
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>
#include "qcustomplot.h"

QT_BEGIN_NAMESPACE

class Ui_SimWindow
{
public:
    QPushButton *closeSimulatorBtn;
    QCustomPlot *plotWidget;
    QWidget *layoutWidget;
    QFormLayout *formLayout;
    QPushButton *btnStart;
    QPushButton *btnStop;
    QPushButton *importCsvBtn;
    QPushButton *btnReset;

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
        layoutWidget = new QWidget(SimWindow);
        layoutWidget->setObjectName("layoutWidget");
        layoutWidget->setGeometry(QRect(30, 510, 168, 56));
        formLayout = new QFormLayout(layoutWidget);
        formLayout->setObjectName("formLayout");
        formLayout->setContentsMargins(0, 0, 0, 0);
        btnStart = new QPushButton(layoutWidget);
        btnStart->setObjectName("btnStart");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, btnStart);

        btnStop = new QPushButton(layoutWidget);
        btnStop->setObjectName("btnStop");

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, btnStop);

        importCsvBtn = new QPushButton(layoutWidget);
        importCsvBtn->setObjectName("importCsvBtn");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, importCsvBtn);

        btnReset = new QPushButton(layoutWidget);
        btnReset->setObjectName("btnReset");

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, btnReset);


        retranslateUi(SimWindow);

        QMetaObject::connectSlotsByName(SimWindow);
    } // setupUi

    void retranslateUi(QDialog *SimWindow)
    {
        SimWindow->setWindowTitle(QCoreApplication::translate("SimWindow", "Dialog", nullptr));
        closeSimulatorBtn->setText(QCoreApplication::translate("SimWindow", "Zamknij", nullptr));
        btnStart->setText(QCoreApplication::translate("SimWindow", "START", nullptr));
        btnStop->setText(QCoreApplication::translate("SimWindow", "STOP", nullptr));
        importCsvBtn->setText(QCoreApplication::translate("SimWindow", "Dane z CSV", nullptr));
        btnReset->setText(QCoreApplication::translate("SimWindow", "Reset", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SimWindow: public Ui_SimWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SIMWINDOW_H
