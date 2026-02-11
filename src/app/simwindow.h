#ifndef SIMWINDOW_H
#define SIMWINDOW_H

#include <QDialog>
#include "components/chartslot.h"
#include "infowindow.h"
#include <QList>

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

    void setupLayout();

signals:
    void dataImportRequested(int slotIndex, const QString &filePath);
    void startRequested(int slotIndex);
    void stopRequested(int slotIndex);
    void resetRequested(int slotIndex);

public slots:
    void onNewData(int slotIndex, double time, double value);
    void onStatsReceived(const SimulationStats & stats);
    void onSlotAddChartRequested(int slotIndex);
    void onSlotCsvLoadRequested(int slotIndex);

private:
    Ui::SimWindow *ui;
    QList <ChartSlot*> m_chartSlots;
    InfoWindow *m_infoWindow = nullptr;
};

#endif
