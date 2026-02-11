#include "simwindow.h"
#include "ui_simwindow.h"
#include "qcustomplot.h"
#include <QDebug> // Do logowania błędów
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>



SimWindow::SimWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SimWindow)
   {
    ui->setupUi(this);

    m_infoWindow = new InfoWindow(this);
    

    //inicjalizacja siatki wykresow
    setupLayout();





}

SimWindow::~SimWindow()
{
    delete ui;
}

void SimWindow::setupLayout()
{
    //Pobieramy wskaznik do grid layoutu, ktory jest wewnatrz srcoll area

    QGridLayout *grid = qobject_cast<QGridLayout*> (ui->chartsContainer->layout());

    if(!grid){
        grid = new QGridLayout(ui->chartsContainer);
        ui->chartsContainer->setLayout(grid);
    }

    m_chartSlots.clear();

    const int COLS = 2; //ilosc kolumn
    const int ROWS = 2; //ilosc wierszy

    int slotCounter = 0;

    for (int row = 0; row < ROWS; ++ row){
        for (int col = 0; col < COLS; ++ col) {
            ChartSlot* slot = new ChartSlot(slotCounter, this);

            connect(slot, &ChartSlot::addChartRequested, this, &SimWindow::onSlotAddChartRequested);
            connect(slot, &ChartSlot::csvLoadRequested, this, &SimWindow::onSlotCsvLoadRequested);
            connect(slot, &ChartSlot::startRequested, this, [this]() { emit startRequested(); });
            connect(slot, &ChartSlot::stopRequested, this, [this]() { emit stopRequested(); });
            connect(slot, &ChartSlot::resetRequested, this, [this]() { emit resetRequested(); });

            m_chartSlots.append(slot);
            grid->addWidget(slot, row, col);

            slotCounter++;
        }

    }
}

void SimWindow::onNewData(double time, double value)
{
    double timeInSeconds = time / 1000.0;

    //przekazujemy dane do wszystkich aktywnych slotów
    for (auto slot : m_chartSlots){
        slot->addDataPoint(timeInSeconds, value);
    }

}

void SimWindow::handleImportCsv()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Importuj Dane CSV"), "", tr("CSV Files(*.csv);;AllFiles(*)")); // otwórz okno dialogowe

    if(fileName.isEmpty()) //jezeli uzytkownik anuluje
        return;

    for (auto slot : m_chartSlots){
        //czyścimy dane, ale nie zamykamy wykresu ( nie robimy reset() do przycisku)
        //Trzeba dodać metodę clearData() do chartSlot lub uzyc publicznego dostępu
        // w kodzie w ChartSlot::displayChart jest czyszczenie wiec mozna uzyć
        if(!slot->isEmpty()){
            slot->displayChart(1, "Symulacja temperatury");
        }
    }
    emit dataImportRequested(fileName); // wyemituj sygnał do controllera
}


void SimWindow::onStatsReceived(const SimulationStats &stats)
{
    if (!m_infoWindow) return;

    m_infoWindow->updateStats(stats);
    m_infoWindow->show();
    m_infoWindow->raise();
    m_infoWindow->activateWindow();

}

void SimWindow::onSlotAddChartRequested(int slotIndex)
{
    if (slotIndex >= 0 && slotIndex < m_chartSlots.size()){
        ChartSlot* slot = m_chartSlots[slotIndex];
        slot->displayChart(slotIndex, "Wykres " + QString::number(slotIndex + 1));
    }
}

void SimWindow::onSlotCsvLoadRequested(int slotIndex)
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Importuj Dane CSV"), "", tr("CSV Files(*.csv);;All Files(*)"));

    if(fileName.isEmpty())
        return;

    if (slotIndex >= 0 && slotIndex < m_chartSlots.size()){
        ChartSlot* slot = m_chartSlots[slotIndex];
        slot->displayChart(slot->getTrendid(), QFileInfo(fileName).fileName());
    }

    emit dataImportRequested(fileName);
}

























