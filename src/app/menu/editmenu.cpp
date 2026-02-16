#include "editmenu.h"

EditMenu::EditMenu(QWidget *parent)
    : QMenu(tr("Edycja"), parent)
{
    QAction *actionLimits = addAction(tr("Dodaj limity"));
    connect(actionLimits, &QAction::triggered, this, &EditMenu::limitsDialogRequested);
}
