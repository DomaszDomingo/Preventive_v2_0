#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "controller/simulationcontroller.h"
#include "menu/filemenu.h"
#include "menu/editmenu.h"
#include "menu/aboutmenu.h"
#include "dialogs/limitsdialog.h"
#include "analysis/pythonanalysisrunner.h"
#include "qcustomplot.h"
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenuBar>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_controller = new SimulationController(this);
    m_infoWindow = new InfoWindow(this);
    m_pythonRunner = new PythonAnalysisRunner(this);

    setupMenus();
    setupLayout();

    // Sygnały kontroler -> okno główne
    connect(m_controller, &SimulationController::newValueProduced, this, &MainWindow::onNewData);
    connect(m_controller, &SimulationController::statsReady, this, &MainWindow::onStatsReceived);
    connect(m_pythonRunner, &PythonAnalysisRunner::analysisFinished, this, &MainWindow::onAnalysisFinished);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupMenus()
{
    m_fileMenu = new FileMenu(this);
    m_editMenu = new EditMenu(this);
    m_aboutMenu = new AboutMenu(this);

    ui->menubar->addMenu(m_fileMenu);
    ui->menubar->addMenu(m_editMenu);
    ui->menubar->addMenu(m_aboutMenu);

    connect(m_fileMenu, &FileMenu::closeRequested, this, &QMainWindow::close);
    connect(m_editMenu, &EditMenu::limitsDialogRequested, this, &MainWindow::onLimitsDialogRequested);
    connect(m_editMenu, &EditMenu::pythonAnalysisTestRequested, this, &MainWindow::onPythonAnalysisTestRequested);
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
            connect(slot, &ChartSlot::speedChanged, m_controller, &SimulationController::setSimulationSpeed);

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

void MainWindow::onLimitsDialogRequested()
{
    if (!m_limitsDialog) {
        m_limitsDialog = new LimitsDialog(m_chartSlots.size(), this);
        connect(m_limitsDialog, &LimitsDialog::limitsApplied, this, &MainWindow::onLimitsApplied);
        connect(m_limitsDialog, &LimitsDialog::limitsVisibilityChanged, this, &MainWindow::onLimitsVisibilityChanged);
    }

    // Wypełnij aktualne limity i widoczność
    for (int i = 0; i < m_chartSlots.size(); ++i) {
        if (m_chartSlots[i]->hasLimits()) {
            m_limitsDialog->setLimits(i, m_chartSlots[i]->limitMin(), m_chartSlots[i]->limitMax());
        }
        m_limitsDialog->setLimitsVisible(i, m_chartSlots[i]->limitsVisible());
    }

    m_limitsDialog->show();
    m_limitsDialog->raise();
    m_limitsDialog->activateWindow();
}

void MainWindow::onLimitsApplied(int slotIndex, double minVal, double maxVal)
{
    if (slotIndex >= 0 && slotIndex < m_chartSlots.size()) {
        m_chartSlots[slotIndex]->setLimits(minVal, maxVal);
    }
}

void MainWindow::onLimitsVisibilityChanged(int slotIndex, bool visible)
{
    if (slotIndex >= 0 && slotIndex < m_chartSlots.size()) {
        m_chartSlots[slotIndex]->setLimitsVisible(visible);
    }
}


//Testowy most C++ <->Python: pozwala uruchomic run_analysis.py na dowolnym pliku CSV i zobaczyć sparsowany wynik,
//aby zweryfikowac, że cała droga (QProcess->stdout -> AnalysisResult::fromJson) działa poprawnie.

void MainWindow::onPythonAnalysisTestRequested()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Wybierz plik danych"), "", tr("CSV Files (*.csv);;All Files(*)"));

    if (fileName.isEmpty())
        return;

    bool ok = false;
    QString column = QInputDialog::getText(this, tr("Kolumna wejściowa"), tr("Nazwy kolumny(po normalizacji, malymi literami):"), QLineEdit::Normal, "temperature_c", &ok);

    if(!ok || column.isEmpty())
        return;

    int slotIndex = QInputDialog::getInt(this, tr("Wybór wykresu"),  tr("Numer slotu (0-%1):").arg(m_chartSlots.size() - 1), 0, 0, m_chartSlots.size() - 1, 1, &ok);

    if (!ok)
        return;


    m_pythonAnalysisTargetSlot = slotIndex;
        m_pythonRunner->runAnalysis("kalman",fileName,QStringList() << column);
}



void MainWindow::onAnalysisFinished(const AnalysisResult &result)
{
    if (!result.isSuccess()){
        QMessageBox::warning(this, tr("Analiza Python"), tr ("Błąd: %1").arg(result.errorMessage()));
        return;
    }

    if (m_pythonAnalysisTargetSlot < 0 || m_pythonAnalysisTargetSlot >= m_chartSlots.size())
        return;

    QString title = tr("%1 (Python) - RSME = %2").arg(result.algorithm()).arg(result.metadata().value("rmse").toDouble(), 0 , 'f', 4);

    m_chartSlots[m_pythonAnalysisTargetSlot]->displayAnalysisResult(m_pythonAnalysisTargetSlot, title, result);

    QMessageBox::information(this, tr("AnalizaPython:"), tr("Algotytm: %1\nPunkty filtered: %2\nPunkty forecast: %3\nRSME: %4")
        .arg(result.algorithm())
        .arg(result.filtered().size())
        .arg(result.forecasts().size())
    .arg(result.metadata().value("rmse").toString()));
}



