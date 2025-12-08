#include "chartslot.h"

ChartSlot::ChartSlot(int slotIndex, QWidget *parent)
    : QWidget(parent), m_slotIndex(slotIndex)
{
    //styl ramki granic slotow
    this->setStyleSheet("ChartSlot {border: 1px solid #ccc; background-color: #f9f9f9; margin: 2px;}");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(2,2,2,2);

    m_stack = new QStackedWidget(this);
    mainLayout->addWidget(m_stack);

    //strona1: Stan pusty(przycisk dodaj)

    m_PageEmpty = new QWidget(this);
    QVBoxLayout *emptyLayout = new QVBoxLayout(m_PageEmpty);
    m_btnAdd = new QPushButton("Dodaj Wykres", m_PageEmpty);
    m_btnAdd->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    emptyLayout->addStretch();
    emptyLayout->addWidget(m_btnAdd, 0, Qt::AlignCenter);
    emptyLayout->addStretch();

    m_stack->addWidget(m_PageEmpty);

    //strona2: aktywny wykres
    m_pageChart = new QWidget(this);
    QVBoxLayout *chartLayout = new QVBoxLayout(m_pageChart);
    chartLayout->setContentsMargins(0,0,0,0);

    m_lblTitle = new QLabel ("Tytuł Wykresu", m_pageChart);
    m_lblTitle->setAlignment(Qt::AlignCenter);
    m_lblTitle->setStyleSheet("font-weight: bold; color: #333;");

    m_plot = new QCustomPlot(m_pageChart);

    //konfiguracja domyslna wykresu
    m_plot->addGraph();
    m_plot->xAxis->setLabel("Czas (s)");
    m_plot->yAxis->setLabel("Wartość");
    m_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    chartLayout->addWidget(m_lblTitle);
    chartLayout->addWidget(m_plot);

    m_stack->addWidget(m_pageChart);

    connect(m_btnAdd, &QPushButton::clicked, this, [this](){
        emit importRequested(m_slotIndex);
    });

    //startujemy od pustego widoku
    reset();

}

void ChartSlot::displayChart(int trendId, const QString &title)
{
    m_trendId = trendId;
    m_lblTitle->setText(title);

    //czyszcenie starego wykresu przy aktywacji
    m_plot->graph(0)->data()->clear();
    m_plot->replot();

    m_stack->setCurrentWidget(m_pageChart);
}

void ChartSlot::reset()
{
    m_trendId = -1;
    if(m_plot) {
        m_plot->graph(0)->data()->clear();
        m_plot->replot();
    }
    m_stack->setCurrentWidget(m_PageEmpty);
}

void ChartSlot::addDataPoint(double time, double value)
{
    //dodajemy dane jeśli slot jest aktywny (ma przypisany trendId)
    if(m_trendId != -1 && m_plot) {
        m_plot->graph(0)->addData(time,value);

        m_plot->xAxis->setRange(time, 5.0, Qt::AlignRight);
        m_plot->yAxis->rescale();

        m_plot->replot(QCustomPlot::rpQueuedReplot);
    }
}
