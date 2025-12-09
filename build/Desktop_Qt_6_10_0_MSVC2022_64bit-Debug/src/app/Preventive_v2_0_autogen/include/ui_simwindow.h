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
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SimWindow
{
public:
    QGridLayout *gridLayout;
    QScrollArea *scrollArea;
    QWidget *chartsContainer;

    void setupUi(QDialog *SimWindow)
    {
        if (SimWindow->objectName().isEmpty())
            SimWindow->setObjectName("SimWindow");
        SimWindow->resize(800, 600);
        SimWindow->setBaseSize(QSize(0, 0));
        gridLayout = new QGridLayout(SimWindow);
        gridLayout->setObjectName("gridLayout");
        scrollArea = new QScrollArea(SimWindow);
        scrollArea->setObjectName("scrollArea");
        scrollArea->setWidgetResizable(true);
        chartsContainer = new QWidget();
        chartsContainer->setObjectName("chartsContainer");
        chartsContainer->setGeometry(QRect(0, 0, 780, 580));
        scrollArea->setWidget(chartsContainer);

        gridLayout->addWidget(scrollArea, 0, 0, 1, 1);


        retranslateUi(SimWindow);

        QMetaObject::connectSlotsByName(SimWindow);
    } // setupUi

    void retranslateUi(QDialog *SimWindow)
    {
        SimWindow->setWindowTitle(QCoreApplication::translate("SimWindow", "Dialog", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SimWindow: public Ui_SimWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SIMWINDOW_H
