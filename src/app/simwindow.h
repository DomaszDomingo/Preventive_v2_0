#ifndef SIMWINDOW_H
#define SIMWINDOW_H

#include <QDialog>
#include <QElapsedTimer>

class QCustomPlot;

namespace Ui {
class SimWindow;
}

class SimWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SimWindow(QWidget *parent = nullptr);
    ~SimWindow();
    void setupPlot();//funkcja do inicjalizacji wykresu

public slots:
    void onNewData (double value); // slot na nowe dane
private:
    Ui::SimWindow *ui;
    QElapsedTimer m_plotTimer; //Timer do śledzenia osi czasu X
    QCustomPlot *m_plot;
};

#endif
