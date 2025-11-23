#include "simwindow.h"
#include "ui_simwindow.h"
#include "qcustomplot.h"
#include <QDebug> // Do logowania błędów
#include <QFileDialog>
#include <QMessageBox>

SimWindow::SimWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SimWindow)
    , m_plot(nullptr) // Zawsze inicjalizuj wskaźniki
{
    ui->setupUi(this);

    // --- KLUCZOWA POPRAWKA ---
    // ui->plotWidget jest typu QWidget*.
    // Musimy go bezpiecznie zrzutować na QCustomPlot*, który został "promowany".

    m_plot = qobject_cast<QCustomPlot*>(ui->plotWidget);

    if (!m_plot) {
        // Jeśli m_plot jest nullptr, to znaczy, że nazwa w .ui jest zła LUB promocja się nie powiodła
        qCritical() << "KRYTYCZNY BŁĄD: Nie można znaleźć lub zrzutować ui->plotWidget na QCustomPlot!";
        return;
    }
    // --- KONIEC POPRAWKI ---

    connect (ui->closeSimulatorBtn, &QPushButton::clicked, this, &SimWindow::close);

    connect (ui->importCsvBtn, &QPushButton::clicked, this, &SimWindow::handleImportCsv);

    // Inicjalizujemy wykres
    setupPlot();
}

SimWindow::~SimWindow()
{
    delete ui;
}

void SimWindow::setupPlot()
{
    // Jeśli m_plot jest nieprawidłowy, nie rób nic
    if (!m_plot) return;

    // Od teraz używamy m_plot, a NIE ui->plotWidget

    //Dodanie nowego wykresu (graph) do widgetu
    m_plot->addGraph();

    //ustawienie osi
    m_plot->xAxis->setLabel ("Czas (s)");
    m_plot->yAxis->setLabel ("Temperatura (°C)");

    //włączenie interakcji (zoom, przesuwanie)
    m_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    //uruchomienie timera do osi x
    m_plotTimer.start();
}

void SimWindow::onNewData(double value)
{
    // Jeśli m_plot jest nieprawidłowy, nie rób nic
    if (!m_plot) return;

    //Pobranie czasu jaki upłynał w sekundach
    double time = m_plotTimer.elapsed() / 1000.0;

    //dodanie punktu (x,y) do wykresu
    m_plot->graph(0)->addData(time, value);

    // --- POPRAWKA LOGIKI WYKRESU (z poprzedniej analizy) ---
    // Wyrównujemy do 'time' (prawa krawędź) i pokazujemy 5 sekund wstecz
    m_plot->xAxis->setRange(time, 5.0, Qt::AlignRight);

    // Oś Y nadal skalujemy automatycznie, aby dopasować do widocznych danych
    m_plot->yAxis->rescale();

    //odswiez wykres
    m_plot->replot(QCustomPlot::rpQueuedReplot);
}

void SimWindow::handleImportCsv()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Importuj Dane CSV"), "", tr("CSV Files(*.csv);;AllFiles(*)")); // otwórz okno dialogowe

    if(fileName.isEmpty()) //jezeli uzytkownik anuluje
        return;

    emit dataImportRequested(fileName); // wyemituj sygnał do controllera

    //przygotowanie wykresu na nowe dane
    m_plot->graph(0)->data()->clear();
    m_plot->replot();


}

























