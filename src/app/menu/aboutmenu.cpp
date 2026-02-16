#include "aboutmenu.h"
#include <QMessageBox>

AboutMenu::AboutMenu(QWidget *parent)
    : QMenu(tr("O aplikacji"), parent)
{
    QAction *actionOProgramie = addAction(tr("O programie"));
    connect(actionOProgramie, &QAction::triggered, this, &AboutMenu::showAboutDialog);
}

void AboutMenu::showAboutDialog()
{
    QMessageBox::about(parentWidget(), tr("O programie"),
        tr("Preventive v2.0\n\nAplikacja do symulacji i wizualizacji danych."));
}
