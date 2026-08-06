#ifndef EDITMENU_H
#define EDITMENU_H

#include <QMenu>

class EditMenu : public QMenu
{
    Q_OBJECT

public:
    explicit EditMenu(QWidget *parent = nullptr);

signals:
    void limitsDialogRequested();
    void pythonAnalysisTestRequested();
};

#endif // EDITMENU_H
