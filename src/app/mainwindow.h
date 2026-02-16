#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include "components/chartslot.h"
#include "infowindow.h"

class SimulationController;
class FileMenu;
class EditMenu;
class AboutMenu;

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
    void onNewData(int slotIndex, double time, double value);
    void onStatsReceived(const SimulationStats &stats);
    void onSlotAddChartRequested(int slotIndex);
    void onSlotCsvLoadRequested(int slotIndex);

private:
    void setupMenus();
    void setupLayout();

    Ui::MainWindow *ui;
    SimulationController *m_controller;
    QList<ChartSlot*> m_chartSlots;
    InfoWindow *m_infoWindow = nullptr;

    FileMenu *m_fileMenu;
    EditMenu *m_editMenu;
    AboutMenu *m_aboutMenu;
};

#endif // MAINWINDOW_H
