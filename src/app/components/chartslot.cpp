#include "chartslot.h"

ChartSlot::ChartSlot(int slotIndex, QWidget *parent)
    : QWidget(parent), m_slotIndex(slotIndex)
{
    //styl ramki granic slotow
    this->setStyleSheet("ChartSlot {border: 1px solid #aaa; border-radius: 4px; background-color: #f0f0f0; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(2,2,2,2);

    m_stack = new QStackedWidget(this);
    mainLayout->addWidget(m_stack);

    //strona1: Stan pusty(przycisk dodaj)

    m_PageEmpty = new QWidget(this);
    QVBoxLayout *emptyLayout = new QVBoxLayout(m_PageEmpty);
    m_btnAdd = new QPushButton("Dodaj Wykres", m_PageEmpty);
    m_btnAdd->setStyleSheet("padding: 10px; font-weight: bold;");

    emptyLayout->addStretch();
    emptyLayout->addWidget(m_btnAdd, 0, Qt::AlignCenter);
    emptyLayout->addStretch();
    m_stack->addWidget(m_PageEmpty);

    m_stack->addWidget(m_PageEmpty);

    //strona2: wykres + sterowanie
    m_pageChart = new QWidget(this);
    QVBoxLayout *chartLayout = new QVBoxLayout(m_pageChart);
    chartLayout->setContentsMargins(0,0,0,0);

    //tytul
    m_lblTitle = new QLabel ("Tytuł Wykresu", m_pageChart);
    m_lblTitle->setAlignment(Qt::AlignCenter);
    m_lblTitle->setStyleSheet("font-weight: bold; color: #333; border: none;");
    chartLayout->addWidget(m_lblTitle);

    //wykres
    m_plot = new QCustomPlot(m_pageChart);
    m_plot->addGraph();
    m_plot->xAxis->setLabel("Czas (s)");
    m_plot->yAxis->setLabel("Wartość");
    m_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    m_plot->setMinimumHeight(200);//Minimalna wysokość, zeby scroll area wiedzialo kiedy przewijac
    chartLayout->addWidget(m_plot, 1);

    //panel przyciskow
    QWidget *controlPanel = new QWidget(m_pageChart);
    controlPanel->setStyleSheet("border: none"); // zeby nie dziedziczyl ramki slotu
    QHBoxLayout *btnLayout = new QHBoxLayout(controlPanel);
    btnLayout->setContentsMargins(0,0,0,0);

    m_btnStart = new QPushButton("Start", controlPanel);
    m_btnStop = new QPushButton("Stop", controlPanel);
    m_btnReset = new QPushButton("Reset", controlPanel);
    m_btnCsv = new QPushButton("Ładuj CSV", controlPanel);

    btnLayout->addWidget(m_btnStart);
    btnLayout->addWidget(m_btnStop);
    btnLayout->addWidget(m_btnReset);
    btnLayout->addWidget(m_btnCsv);

    chartLayout->addWidget(controlPanel);
    m_stack->addWidget(m_pageChart);

    connect(m_btnAdd, &QPushButton::clicked, this, [this](){
        emit importRequested(m_slotIndex);
    });

    //przekazywanie sygnałów sterujacych
    connect(m_btnStart, &QPushButton::clicked, this, [this]() { emit startRequested(m_slotIndex); });
    connect(m_btnStop, &QPushButton::clicked, this, [this]() { emit stopRequested(m_slotIndex); });
    connect(m_btnReset, &QPushButton::clicked, this, [this]() {
        if (m_plot){
            m_plot->graph(0)->data()->clear();
            m_plot->replot();
        }
        emit resetRequested(m_slotIndex);
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
