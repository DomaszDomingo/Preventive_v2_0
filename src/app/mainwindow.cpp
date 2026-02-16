#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "controller/simulationcontroller.h"
#include "qcustomplot.h"
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_controller = new SimulationController(this);
    m_infoWindow = new InfoWindow(this);

    setupLayout();

    // Sygnały kontroler -> okno główne
    connect(m_controller, &SimulationController::newValueProduced, this, &MainWindow::onNewData);
    connect(m_controller, &SimulationController::statsReady, this, &MainWindow::onStatsReceived);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Tworzy siatkę 2x2 slotów wykresów (ChartSlot) w kontenerze scroll area.
// Każdy slot otrzymuje unikalny indeks (0-3) i podłącza swoje sygnały
// (dodaj wykres, ładuj CSV, start, stop, reset) do odpowiednich slotów/sygnałów.
// Dzięki temu każdy wykres działa niezależnie - slotIndex jest przekazywany
// przez cały łańcuch sygnałów aż do kontrolera.
void MainWindow::setupLayout()
{
    QGridLayout *grid = qobject_cast<QGridLayout*>(ui->chartsContainer->layout());

    if (!grid) {
        grid = new QGridLayout(ui->chartsContainer);
        ui->chartsContainer->setLayout(grid);
    }

    m_chartSlots.clear();

    const int COLS = 2;
    const int ROWS = 2;
    int slotCounter = 0;

    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            ChartSlot *slot = new ChartSlot(slotCounter, this);

            connect(slot, &ChartSlot::addChartRequested, this, &MainWindow::onSlotAddChartRequested);
            connect(slot, &ChartSlot::csvLoadRequested, this, &MainWindow::onSlotCsvLoadRequested);
            connect(slot, &ChartSlot::startRequested, m_controller, &SimulationController::startSimulation);
            connect(slot, &ChartSlot::stopRequested, m_controller, &SimulationController::stopSimulation);
            connect(slot, &ChartSlot::resetRequested, m_controller, &SimulationController::resetSimulation);

            m_chartSlots.append(slot);
            grid->addWidget(slot, row, col);
            slotCounter++;
        }
    }
}

// Odbiera nowy punkt danych z kontrolera i kieruje go do konkretnego slotu.
// Przelicza czas z milisekund na sekundy przed przekazaniem do wykresu.
void MainWindow::onNewData(int slotIndex, double time, double value)
{
    double timeInSeconds = time / 1000.0;
    if (slotIndex >= 0 && slotIndex < m_chartSlots.size()) {
        m_chartSlots[slotIndex]->addDataPoint(timeInSeconds, value);
    }
}

// Odbiera statystyki z kontrolera po załadowaniu danych CSV
// i wyświetla je w oknie InfoWindow (liczba próbek, min/max, średnia itd.).
void MainWindow::onStatsReceived(const SimulationStats &stats)
{
    if (!m_infoWindow) return;

    m_infoWindow->updateStats(stats);
    m_infoWindow->show();
    m_infoWindow->raise();
    m_infoWindow->activateWindow();
}

// Obsługuje kliknięcie "Dodaj Wykres" - przełącza slot z widoku pustego
// na widok wykresu z domyślnym tytułem "Wykres N". Nie ładuje danych -
// użytkownik musi osobno kliknąć "Ładuj CSV".
void MainWindow::onSlotAddChartRequested(int slotIndex)
{
    if (slotIndex >= 0 && slotIndex < m_chartSlots.size()) {
        ChartSlot *slot = m_chartSlots[slotIndex];
        slot->displayChart(slotIndex, "Wykres " + QString::number(slotIndex + 1));
    }
}

// Obsługuje kliknięcie "Ładuj CSV" - otwiera dialog wyboru pliku,
// ustawia tytuł wykresu na nazwę pliku i emituje sygnał do kontrolera
// w celu załadowania danych do symulatora przypisanego do tego konkretnego slotu.
void MainWindow::onSlotCsvLoadRequested(int slotIndex)
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Importuj Dane CSV"), "", tr("CSV Files(*.csv);;All Files(*)"));

    if (fileName.isEmpty())
        return;

    if (slotIndex >= 0 && slotIndex < m_chartSlots.size()) {
        ChartSlot *slot = m_chartSlots[slotIndex];
        slot->displayChart(slotIndex, QFileInfo(fileName).fileName());
    }

    m_controller->loadNewData(slotIndex, fileName);
}
