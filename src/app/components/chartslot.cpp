#include "chartslot.h"
#include "../datamodel/analysisresult.h"


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

    //strona2: wykres + sterowanie
    m_pageChart = new QWidget(this);
    QVBoxLayout *chartLayout = new QVBoxLayout(m_pageChart);
    chartLayout->setContentsMargins(0,0,0,0);

    //tytul
    m_lblTitle = new QLabel("Tytuł Wykresu", m_pageChart);
    m_lblTitle->setAlignment(Qt::AlignCenter);
    m_lblTitle->setStyleSheet("font-weight: bold; color: #333; border: none;");
    chartLayout->addWidget(m_lblTitle);

    //wykres
    m_plot = new QCustomPlot(m_pageChart);
    m_plot->addGraph();
    m_plot->addGraph();
    m_plot->addGraph();

    m_plot->graph(1)->setPen(QPen(QColor(0,120,200),2));
    m_plot->graph(1)->setName("Filtrowana (Kalman)");

    m_plot->graph(2)->setPen(QPen(QColor(200,100,0),2, Qt::DashLine));
    m_plot->graph(2)->setName("Prognoza");

    m_plot->xAxis->setLabel("Czas (s)");
    m_plot->yAxis->setLabel("Wartość");

    m_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems);
    m_plot->setMinimumHeight(200);

    //jeśli zakres osi X zmienia się, czyli uzytkownik przeciągnął lub przyblizyl oddalil wylacz auto follow

    connect (m_plot->xAxis, QOverload<const QCPRange &>::of(&QCPAxis::rangeChanged), this, [this](const QCPRange &){
        if (!m_isAutoScrolling)
            m_autoFollow = false;
    });

    //menu kontekstowe na wykresie (usuwanie wykresu i kursorów)
    m_plot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_plot, &QCustomPlot::customContextMenuRequested, this, [this](const QPoint &pos){
        QMenu menu(this);
        QAction *removeCursorsAction = nullptr;
        if (m_cursorLineA || m_cursorLineB) {
            removeCursorsAction = menu.addAction("Usuń kursory");
        }
        QAction *removeChartAction = menu.addAction("Usuń wykres");
        QAction *followAction = menu.addAction("Powrot do początku wykr.");

        QAction *selected = menu.exec(m_plot->mapToGlobal(pos));
        if (selected == removeChartAction) {
            emit resetRequested(m_slotIndex);
            reset();
        } else if (removeCursorsAction && selected == removeCursorsAction) {
            removeCursors();
        } else if (selected == followAction){
            m_autoFollow = true;
            if (m_plot->graph(0)->data()->size() > 0){
                double lastKey = (*(m_plot->graph(0)->data()->constEnd() - 1)).key;
                m_isAutoScrolling = true;
                m_plot->xAxis->setRange(lastKey, 5.0, Qt::AlignCenter);
                m_plot->graph(0)->rescaleValueAxis(false,true);
                m_isAutoScrolling = false;
                m_plot->replot();
            }

        }
    });

    // Dwuklik na wykresie stawia kursor A lub B naprzemiennie
    connect(m_plot, &QCustomPlot::mouseDoubleClick, this, [this](QMouseEvent *event){
        if (event->button() == Qt::LeftButton) {
            double x = m_plot->xAxis->pixelToCoord(event->position().x());
            placeCursor(x);
        }
    });

    // Rozpoczęcie przeciągania kursora - sprawdza czy kliknięto na linię kursora
    connect(m_plot, &QCustomPlot::mousePress, this, [this](QMouseEvent *event){
        if (event->button() == Qt::LeftButton) {
            QCPAbstractItem *item = m_plot->itemAt(event->position(), true);
            QCPItemStraightLine *line = qobject_cast<QCPItemStraightLine*>(item);
            if (line && (line == m_cursorLineA || line == m_cursorLineB)) {
                m_draggedCursor = line;
            }
        }
    });

    // Przeciąganie kursora - aktualizuje pozycję linii, tracera i etykiety
    connect(m_plot, &QCustomPlot::mouseMove, this, [this](QMouseEvent *event){
        if (m_draggedCursor) {
            double x = m_plot->xAxis->pixelToCoord(event->position().x());
            m_draggedCursor->point1->setCoords(x, 0);
            m_draggedCursor->point2->setCoords(x, 1);

            if (m_draggedCursor == m_cursorLineA && m_cursorTracerA) {
                m_cursorTracerA->setGraphKey(x);
            } else if (m_draggedCursor == m_cursorLineB && m_cursorTracerB) {
                m_cursorTracerB->setGraphKey(x);
            }

            updateCursorLabels();
            m_plot->replot(QCustomPlot::rpQueuedReplot);
        }
    });

    // Zakończenie przeciągania kursora
    connect(m_plot, &QCustomPlot::mouseRelease, this, [this](QMouseEvent *){
        m_draggedCursor = nullptr;
    });

    chartLayout->addWidget(m_plot, 1);

    //panel przyciskow
    QWidget *controlPanel = new QWidget(m_pageChart);
    controlPanel->setObjectName("controlPanel");
    controlPanel->setStyleSheet("#controlPanel { border: none; }");
    QHBoxLayout *btnLayout = new QHBoxLayout(controlPanel);
    btnLayout->setContentsMargins(0,0,0,0);

    QString btnStyle = "QPushButton { padding: 5px 12px; border: 1px solid #888; border-radius: 3px; background-color: #e0e0e0; }"
                       "QPushButton:hover { background-color: #d0d0d0; }"
                       "QPushButton:pressed { background-color: #c0c0c0; }";

    m_btnStart = new QPushButton("Start", controlPanel);
    m_btnStop = new QPushButton("Stop", controlPanel);
    m_btnReset = new QPushButton("Reset", controlPanel);
    m_btnCsv = new QPushButton("Ładuj CSV", controlPanel);

    m_btnStart->setStyleSheet(btnStyle);
    m_btnStop->setStyleSheet(btnStyle);
    m_btnReset->setStyleSheet(btnStyle);
    m_btnCsv->setStyleSheet(btnStyle);

    btnLayout->addWidget(m_btnStart);
    btnLayout->addWidget(m_btnStop);
    btnLayout->addWidget(m_btnReset);
    btnLayout->addWidget(m_btnCsv);

    chartLayout->addWidget(controlPanel);
    QWidget *speedPanel = new QWidget (m_pageChart);
    speedPanel->setObjectName("speedPanel");
    speedPanel->setStyleSheet("#speedPanel { border: none; }");
    QHBoxLayout *speedLayout = new QHBoxLayout(speedPanel);
    speedLayout->setContentsMargins(0,0,0,0);

    m_speedLabel = new QLabel("Predkość: 1x", speedPanel);
    m_speedSlider = new QSlider (Qt::Horizontal, speedPanel);
    m_speedSlider->setRange(1,100);
    m_speedSlider->setValue(1);


    speedLayout->addWidget(m_speedLabel);
    speedLayout->addWidget(m_speedSlider,1);

    chartLayout->addWidget(speedPanel);

    connect(m_speedSlider, &QSlider::valueChanged, this, [this](int value){
        m_speedLabel->setText(QString("Predkość: %1x").arg(value));
        emit speedChanged(m_slotIndex, static_cast<double>(value));

    });


    m_stack->addWidget(m_pageChart);

    //"Dodaj Wykres" - pokazuje pusty wykres
    connect(m_btnAdd, &QPushButton::clicked, this, [this](){
        emit addChartRequested(m_slotIndex);
    });

    //przekazywanie sygnałów sterujacych
    connect(m_btnStart, &QPushButton::clicked, this, [this]() { emit startRequested(m_slotIndex); });
    connect(m_btnStop, &QPushButton::clicked, this, [this]() { emit stopRequested(m_slotIndex); });
    connect(m_btnReset, &QPushButton::clicked, this, [this]() {
        if (m_plot){
            m_plot->graph(0)->data()->clear();
            m_plot->graph(1)->data()->clear();
            m_plot->graph(2)->data()->clear();
            m_plot->replot();
        }
        m_autoFollow = true;
        emit resetRequested(m_slotIndex);
    });
    connect(m_btnCsv, &QPushButton::clicked, this, [this]() { emit csvLoadRequested(m_slotIndex); });

    //startujemy od pustego widoku
    reset();
}

// Stawia kursor A lub B (naprzemiennie) w podanej pozycji X.
// Tworzy pionową linię, tracer snapujący do wykresu i etykietę z wartościami.
// Jeśli kursor już istnieje, przesuwa go zamiast tworzyć nowy.
void ChartSlot::placeCursor(double xCoord)
{
    if (m_nextCursorIsA) {
        // Tworzenie lub przesuwanie kursora A
        if (!m_cursorLineA) {
            m_cursorLineA = new QCPItemStraightLine(m_plot);
            m_cursorLineA->setPen(QPen(QColor(0, 160, 0), 2, Qt::DashLine));
            m_cursorLineA->setSelectable(true);

            m_cursorTracerA = new QCPItemTracer(m_plot);
            m_cursorTracerA->setGraph(m_plot->graph(0));
            m_cursorTracerA->setStyle(QCPItemTracer::tsCircle);
            m_cursorTracerA->setSize(8);
            m_cursorTracerA->setPen(QPen(QColor(0, 160, 0), 2));
            m_cursorTracerA->setBrush(QBrush(QColor(0, 160, 0)));
            m_cursorTracerA->setInterpolating(true);

            m_cursorLabelA = new QCPItemText(m_plot);
            m_cursorLabelA->position->setTypeX(QCPItemPosition::ptPlotCoords);
            m_cursorLabelA->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
            m_cursorLabelA->setPositionAlignment(Qt::AlignTop | Qt::AlignHCenter);
            m_cursorLabelA->setFont(QFont("Arial", 9));
            m_cursorLabelA->setColor(QColor(0, 160, 0));
            m_cursorLabelA->setPadding(QMargins(4, 2, 4, 2));
            m_cursorLabelA->setBrush(QBrush(QColor(255, 255, 255, 200)));
            m_cursorLabelA->setPen(QPen(QColor(0, 160, 0)));
        }

        m_cursorLineA->point1->setCoords(xCoord, 0);
        m_cursorLineA->point2->setCoords(xCoord, 1);
        m_cursorTracerA->setGraphKey(xCoord);

    } else {
        // Tworzenie lub przesuwanie kursora B
        if (!m_cursorLineB) {
            m_cursorLineB = new QCPItemStraightLine(m_plot);
            m_cursorLineB->setPen(QPen(QColor(200, 0, 0), 2, Qt::DashLine));
            m_cursorLineB->setSelectable(true);

            m_cursorTracerB = new QCPItemTracer(m_plot);
            m_cursorTracerB->setGraph(m_plot->graph(0));
            m_cursorTracerB->setStyle(QCPItemTracer::tsCircle);
            m_cursorTracerB->setSize(8);
            m_cursorTracerB->setPen(QPen(QColor(200, 0, 0), 2));
            m_cursorTracerB->setBrush(QBrush(QColor(200, 0, 0)));
            m_cursorTracerB->setInterpolating(true);

            m_cursorLabelB = new QCPItemText(m_plot);
            m_cursorLabelB->position->setTypeX(QCPItemPosition::ptPlotCoords);
            m_cursorLabelB->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
            m_cursorLabelB->setPositionAlignment(Qt::AlignTop | Qt::AlignHCenter);
            m_cursorLabelB->setFont(QFont("Arial", 9));
            m_cursorLabelB->setColor(QColor(200, 0, 0));
            m_cursorLabelB->setPadding(QMargins(4, 2, 4, 2));
            m_cursorLabelB->setBrush(QBrush(QColor(255, 255, 255, 200)));
            m_cursorLabelB->setPen(QPen(QColor(200, 0, 0)));
        }

        m_cursorLineB->point1->setCoords(xCoord, 0);
        m_cursorLineB->point2->setCoords(xCoord, 1);
        m_cursorTracerB->setGraphKey(xCoord);

        // Tworzenie etykiety delta przy pierwszym postawieniu obu kursorów
        if (!m_deltaLabel && m_cursorLineA) {
            m_deltaLabel = new QCPItemText(m_plot);
            m_deltaLabel->position->setTypeX(QCPItemPosition::ptPlotCoords);
            m_deltaLabel->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
            m_deltaLabel->setPositionAlignment(Qt::AlignTop | Qt::AlignHCenter);
            m_deltaLabel->setFont(QFont("Arial", 9, QFont::Bold));
            m_deltaLabel->setColor(Qt::darkBlue);
            m_deltaLabel->setPadding(QMargins(6, 3, 6, 3));
            m_deltaLabel->setBrush(QBrush(QColor(255, 255, 230, 220)));
            m_deltaLabel->setPen(QPen(Qt::darkBlue));
        }
    }

    m_nextCursorIsA = !m_nextCursorIsA;
    updateCursorLabels();
    m_plot->replot();
}

// Aktualizuje tekst etykiet kursorów i deltę na podstawie bieżących pozycji.
// Odczytuje wartość Y z tracera (snap do danych wykresu) i pozycjonuje etykiety
// w górnej części wykresu. Etykieta delta wyświetla różnicę czasu i wartości.
void ChartSlot::updateCursorLabels()
{
    // Y jest w trybie ptAxisRectRatio: 0.0 = góra wykresu, 1.0 = dół wykresu

    if (m_cursorTracerA && m_cursorLabelA) {
        m_cursorTracerA->updatePosition();
        double tA = m_cursorTracerA->position->key();
        double vA = m_cursorTracerA->position->value();
        m_cursorLabelA->setText(QString("A: t=%1 s\nv=%2").arg(tA, 0, 'f', 2).arg(vA, 0, 'f', 2));
        m_cursorLabelA->position->setCoords(tA, 0.03);
    }

    if (m_cursorTracerB && m_cursorLabelB) {
        m_cursorTracerB->updatePosition();
        double tB = m_cursorTracerB->position->key();
        double vB = m_cursorTracerB->position->value();
        m_cursorLabelB->setText(QString("B: t=%1 s\nv=%2").arg(tB, 0, 'f', 2).arg(vB, 0, 'f', 2));
        m_cursorLabelB->position->setCoords(tB, 0.03);
    }

    if (m_deltaLabel && m_cursorTracerA && m_cursorTracerB) {
        double tA = m_cursorTracerA->position->key();
        double vA = m_cursorTracerA->position->value();
        double tB = m_cursorTracerB->position->key();
        double vB = m_cursorTracerB->position->value();
        double midX = (tA + tB) / 2.0;

        m_deltaLabel->setText(QString("Δt=%1 s  Δv=%2")
                                  .arg(qAbs(tB - tA), 0, 'f', 2)
                                  .arg(qAbs(vB - vA), 0, 'f', 2));
        m_deltaLabel->position->setCoords(midX, 0.13);
    }
}

// Usuwa wszystkie kursory (linie, tracery, etykiety, deltę) z wykresu
// i zeruje wskaźniki. Resetuje stan naprzemiennego stawiania kursorów.
void ChartSlot::removeCursors()
{
    if (m_cursorLineA)  { m_plot->removeItem(m_cursorLineA);  m_cursorLineA = nullptr; }
    if (m_cursorTracerA){ m_plot->removeItem(m_cursorTracerA); m_cursorTracerA = nullptr; }
    if (m_cursorLabelA) { m_plot->removeItem(m_cursorLabelA);  m_cursorLabelA = nullptr; }
    if (m_cursorLineB)  { m_plot->removeItem(m_cursorLineB);   m_cursorLineB = nullptr; }
    if (m_cursorTracerB){ m_plot->removeItem(m_cursorTracerB); m_cursorTracerB = nullptr; }
    if (m_cursorLabelB) { m_plot->removeItem(m_cursorLabelB);  m_cursorLabelB = nullptr; }
    if (m_deltaLabel)   { m_plot->removeItem(m_deltaLabel);    m_deltaLabel = nullptr; }

    m_nextCursorIsA = true;
    m_draggedCursor = nullptr;
    m_plot->replot();
}

// Przełącza slot z widoku pustego na widok wykresu.
// Ustawia identyfikator trendu i tytuł, czyści poprzednie dane wykresu.
void ChartSlot::displayChart(int trendId, const QString &title)
{
    m_trendId = trendId;
    m_lblTitle->setText(title);

    m_plot->graph(0)->data()->clear();
    m_plot->graph(1)->data()->clear();
    m_plot->graph(2)->data()->clear();
    m_plot->replot();

    m_autoFollow = true;
    m_stack->setCurrentWidget(m_pageChart);
}

void ChartSlot::setLimits(double min, double max)
{
    m_limitMin = min;
    m_limitMax = max;
    m_hasLimits = true;
    if (m_limitsVisible)
        updateLimitsVisuals();
}

void ChartSlot::setLimitsVisible(bool visible)
{
    m_limitsVisible = visible;
    if (visible && m_hasLimits)
        updateLimitsVisuals();
    else {
        removeLimitsVisuals();
        if (m_plot) m_plot->replot();
    }
}

void ChartSlot::updateLimitsVisuals()
{
    if (!m_plot || !m_hasLimits) return;

    // Usuwamy stare, tworzymy nowe
    removeLimitsVisuals();

    QPen dashPen(QColor(200, 0, 0), 2, Qt::DashLine);
    QBrush zoneBrush(QColor(0, 0, 0, 40));

    // Linia dolna (min)
    m_limitLineMin = new QCPItemStraightLine(m_plot);
    m_limitLineMin->setPen(dashPen);
    m_limitLineMin->point1->setCoords(0, m_limitMin);
    m_limitLineMin->point2->setCoords(1, m_limitMin);

    // Linia górna (max)
    m_limitLineMax = new QCPItemStraightLine(m_plot);
    m_limitLineMax->setPen(dashPen);
    m_limitLineMax->point1->setCoords(0, m_limitMax);
    m_limitLineMax->point2->setCoords(1, m_limitMax);

    // Strefa poniżej min — góra w koordynatach wykresu (limitMin),
    // dół i boki przylegają do krawędzi widocznego obszaru (axisRectRatio)
    m_limitZoneMin = new QCPItemRect(m_plot);
    m_limitZoneMin->setPen(Qt::NoPen);
    m_limitZoneMin->setBrush(zoneBrush);
    m_limitZoneMin->topLeft->setTypeX(QCPItemPosition::ptAxisRectRatio);
    m_limitZoneMin->topLeft->setTypeY(QCPItemPosition::ptPlotCoords);
    m_limitZoneMin->topLeft->setCoords(0, m_limitMin);
    m_limitZoneMin->bottomRight->setTypeX(QCPItemPosition::ptAxisRectRatio);
    m_limitZoneMin->bottomRight->setTypeY(QCPItemPosition::ptAxisRectRatio);
    m_limitZoneMin->bottomRight->setCoords(1, 1);

    // Strefa powyżej max — dół w koordynatach wykresu (limitMax),
    // góra i boki przylegają do krawędzi widocznego obszaru (axisRectRatio)
    m_limitZoneMax = new QCPItemRect(m_plot);
    m_limitZoneMax->setPen(Qt::NoPen);
    m_limitZoneMax->setBrush(zoneBrush);
    m_limitZoneMax->topLeft->setTypeX(QCPItemPosition::ptAxisRectRatio);
    m_limitZoneMax->topLeft->setTypeY(QCPItemPosition::ptAxisRectRatio);
    m_limitZoneMax->topLeft->setCoords(0, 0);
    m_limitZoneMax->bottomRight->setTypeX(QCPItemPosition::ptAxisRectRatio);
    m_limitZoneMax->bottomRight->setTypeY(QCPItemPosition::ptPlotCoords);
    m_limitZoneMax->bottomRight->setCoords(1, m_limitMax);

    m_plot->replot();
}

void ChartSlot::removeLimitsVisuals()
{
    if (!m_plot) return;

    if (m_limitLineMin)  { m_plot->removeItem(m_limitLineMin);  m_limitLineMin = nullptr; }
    if (m_limitLineMax)  { m_plot->removeItem(m_limitLineMax);  m_limitLineMax = nullptr; }
    if (m_limitZoneMin)  { m_plot->removeItem(m_limitZoneMin);  m_limitZoneMin = nullptr; }
    if (m_limitZoneMax)  { m_plot->removeItem(m_limitZoneMax);  m_limitZoneMax = nullptr; }
}

void ChartSlot::reset()
{
    m_trendId = -1;
    if(m_plot) {
        removeCursors();
        removeLimitsVisuals();
        m_plot->graph(0)->data()->clear();
        m_plot->graph(1)->data()->clear();
        m_plot->graph(2)->data()->clear();
        m_plot->replot();
    }
    m_hasLimits = false;
    m_autoFollow= true;
    m_stack->setCurrentWidget(m_PageEmpty);
}

// Dodaje nowy punkt danych do wykresu w czasie rzeczywistym.
// Działa tylko gdy slot jest aktywny (trendId != -1).
// Przesuwa oś X za ostatnim punktem (okno 5s) i automatycznie skaluje oś Y.
void ChartSlot::addDataPoint(double time, double value)
{
    if(m_trendId != -1 && m_plot) {
        m_plot->graph(0)->addData(time,value);

        if(m_autoFollow){
            m_isAutoScrolling = true;
            m_plot->xAxis->setRange(time, 5.0, Qt::AlignCenter);
            m_plot->graph(0)->rescaleValueAxis(false,true);
            m_isAutoScrolling= false;
        }

        m_plot->replot(QCustomPlot::rpQueuedReplot);
    }
}

void ChartSlot::displayAnalysisResult(int trendId, const QString &title, const AnalysisResult &result)
{
    m_trendId = trendId;
    m_lblTitle->setText(title);

    m_plot->graph(0)->data()->clear();
    m_plot->graph(1)->data()->clear();
    m_plot->graph(2)->data()->clear();

    const QList<FilteredPoint> filtered = result.filtered();
    for (const FilteredPoint &point : filtered){
        double t = point.timestamp / 1000.0;
        m_plot->graph(0)->addData(t, point.value);
        m_plot->graph(1)->addData(t, point.predicted);
    }

    const QList<ForecastPoint> forecasts = result.forecasts();
    for (const ForecastPoint &point : forecasts){
        double t = point.timestamp / 1000.0;
        m_plot->graph(2)->addData(t,point.predicted);
    }

    m_plot->xAxis->rescale();
    m_plot->yAxis->rescale();
    m_plot->replot();

    m_stack->setCurrentWidget(m_pageChart);

}


