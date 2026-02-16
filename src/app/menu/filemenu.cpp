#include "filemenu.h"
#include <QKeySequence>

FileMenu::FileMenu(QWidget *parent)
    : QMenu(tr("Plik"), parent)
{
    QAction *actionZamknij = addAction(tr("Zamknij"));
    actionZamknij->setShortcut(QKeySequence(tr("Ctrl+Q")));
    connect(actionZamknij, &QAction::triggered, this, &FileMenu::closeRequested);
}
