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
    void dataImportRequested (const QString &filePath); //sygnał emitowany gdy użytkownik wybierze plik
    void startRequested();
    void stopRequested();
    void resetRequested();

public slots:
    void onNewData (double time, double value); // slot na nowe dane
    void handleImportCsv(); //slot dla przycisku
    void onStatsReceived(const SimulationStats & stats);
    void onSlotAddChartRequested(int slotIndex);
    void onSlotCsvLoadRequested(int slotIndex);
private:
    Ui::SimWindow *ui;
    QList <ChartSlot*> m_chartSlots;
    InfoWindow *m_infoWindow = nullptr;
};

#endif
