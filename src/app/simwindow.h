#ifndef SIMWINDOW_H
#define SIMWINDOW_H

#include <QDialog>
//#include <QElapsedTimer>
#include "infowindow.h"

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

signals:
    void dataImportRequested (const QString &filePath); //sygnał emitowany gdy użytkownik wybierze plik
    void startRequested();
    void stopRequested();
    void resetRequested();

public slots:
    void onNewData (double time, double value); // slot na nowe dane
    void handleImportCsv(); //slot dla przycisku
    void onStatsReceived(const SimulationStats & stats);
private:
    Ui::SimWindow *ui;
    //QElapsedTimer m_plotTimer; //Timer do śledzenia osi czasu X
    QCustomPlot *m_plot;
    InfoWindow *m_infoWindow = nullptr;
};

#endif
