#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>



class SimWindow;
class SimulationController;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_simulatorBtn_clicked();

private:
    Ui::MainWindow *ui;
    SimWindow *m_simWindow = nullptr;
    SimulationController * m_controller;


};
#endif // MAINWINDOW_H
