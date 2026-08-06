#include "editmenu.h"

EditMenu::EditMenu(QWidget *parent)
    : QMenu(tr("Edycja"), parent)
{
    QAction *actionLimits = addAction(tr("Dodaj limity"));
    connect(actionLimits, &QAction::triggered, this, &EditMenu::limitsDialogRequested);

    QAction *actionPythonTest = addAction(tr("Uruchom analize Python (test)"));
    connect(actionPythonTest, &QAction::triggered, this, &EditMenu::pythonAnalysisTestRequested);
}
