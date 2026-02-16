#ifndef ABOUTMENU_H
#define ABOUTMENU_H

#include <QMenu>

class AboutMenu : public QMenu
{
    Q_OBJECT

public:
    explicit AboutMenu(QWidget *parent = nullptr);

private slots:
    void showAboutDialog();
};

#endif // ABOUTMENU_H
