#ifndef CHARTSLOT_H
#define CHARTSLOT_H


#include <QWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include "qcustomplot.h"

class ChartSlot : public QWidget
{
    Q_OBJECT

public:
    explicit ChartSlot(int slotIndex, QWidget *parent = nullptr);

    //przelaczanie widoku na wykres i ustawienie jego tytulu
    void displayChart (int trendId, const QString &title);

    //przelacza widok na przycisk dodaj i czysci dane
    void reset();

    //aktualizacja wewnętrznego wykresu o nowy punkt
    void addDataPoint (double time, double value);

    //gettery
    int getTrendid() const {return m_trendId;}
    int getSlotIndex() const {return m_slotIndex;}
    bool isEmpty() const {return m_trendId == -1;}

signals:
    //sygnal dla (simwindow) ze uzytkownik klinał dodaj w tym slocie
    void importRequested (int slotIndex);
    void startRequested(int slotIndex);
    void stopRequested (int slotIndex);
    void resetRequested(int slotIndex);

private:
    int m_slotIndex;
    int m_trendId = -1; //oznacza ze pusty slot

    //Tu musi byc wskaźnik, poniewaz w cpp jest new QStackedWidget
    QStackedWidget *m_stack;

    //strona1: pusty stan
    QWidget *m_PageEmpty;
    QPushButton *m_btnAdd;

    //strona2: wykres aktywny
    QWidget *m_pageChart;
    QCustomPlot *m_plot;
    QLabel *m_lblTitle;

    QPushButton *m_btnStart;
    QPushButton *m_btnStop;
    QPushButton *m_btnReset;
    QPushButton *m_btnCsv;
};

#endif // CHARTSLOT_H
