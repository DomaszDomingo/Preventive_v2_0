#ifndef FILEMENU_H
#define FILEMENU_H

#include <QMenu>

class FileMenu : public QMenu
{
    Q_OBJECT

public:
    explicit FileMenu(QWidget *parent = nullptr);

signals:
    void closeRequested();
};

#endif // FILEMENU_H
