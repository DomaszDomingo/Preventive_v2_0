#ifndef CHARTSLOT_H
#define CHARTSLOT_H


#include <QWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QMenu>
#include "qcustomplot.h"
#include <QSlider>

class AnalysisResult;

class ChartSlot : public QWidget
{
    Q_OBJECT

public:
    explicit ChartSlot(int slotIndex, QWidget *parent = nullptr);

    //przelaczanie widoku na wykres i ustawienie jego tytulu
    void displayChart (int trendId, const QString &title);

    //wyswietla wynik analizy python (wartosci rzeczywiste, filtrowane, prognoza)
    void displayAnalysisResult (int trendId, const QString &title, const AnalysisResult &result);
    //przelacza widok na przycisk dodaj i czysci dane
    void reset();

    //aktualizacja wewnętrznego wykresu o nowy punkt
    void addDataPoint (double time, double value);

    //limity
    void setLimits(double min, double max);
    void setLimitsVisible(bool visible);
    double limitMin() const { return m_limitMin; }
    double limitMax() const { return m_limitMax; }
    bool hasLimits() const { return m_hasLimits; }
    bool limitsVisible() const { return m_limitsVisible; }

    //gettery
    int getTrendid() const {return m_trendId;}
    int getSlotIndex() const {return m_slotIndex;}
    bool isEmpty() const {return m_trendId == -1;}

signals:
    void addChartRequested(int slotIndex);
    void csvLoadRequested(int slotIndex);
    void startRequested(int slotIndex);
    void stopRequested(int slotIndex);
    void resetRequested(int slotIndex);
    void speedChanged(int slotIndex, double multiplier);


private:
    // Stawia kursor A lub B (naprzemiennie) w podanej pozycji X na wykresie.
    void placeCursor(double xCoord);
    // Aktualizuje tekst etykiet kursorów i deltę na podstawie bieżących pozycji.
    void updateCursorLabels();
    // Usuwa wszystkie kursory z wykresu i zeruje wskaźniki.
    void removeCursors();

    int m_slotIndex;
    int m_trendId = -1; //oznacza ze pusty slot

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
    QSlider *m_speedSlider;
    QLabel *m_speedLabel;

    // Kursory pomiarowe
    QCPItemStraightLine *m_cursorLineA = nullptr;
    QCPItemStraightLine *m_cursorLineB = nullptr;
    QCPItemTracer *m_cursorTracerA = nullptr;
    QCPItemTracer *m_cursorTracerB = nullptr;
    QCPItemText *m_cursorLabelA = nullptr;
    QCPItemText *m_cursorLabelB = nullptr;
    QCPItemText *m_deltaLabel = nullptr;
    bool m_nextCursorIsA = true;
    bool m_autoFollow = true;
    bool m_isAutoScrolling = false;
    QCPItemStraightLine *m_draggedCursor = nullptr;

    // Limity wartości
    double m_limitMin = 0.0;
    double m_limitMax = 0.0;
    bool m_hasLimits = false;
    bool m_limitsVisible = true;

    void updateLimitsVisuals();
    void removeLimitsVisuals();

    QCPItemStraightLine *m_limitLineMin = nullptr;
    QCPItemStraightLine *m_limitLineMax = nullptr;
    QCPItemRect *m_limitZoneMin = nullptr;
    QCPItemRect *m_limitZoneMax = nullptr;
};

#endif // CHARTSLOT_H
