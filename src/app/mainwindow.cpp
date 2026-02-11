#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "simwindow.h"
#include <QDebug>
#include "controller/simulationcontroller.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Utworzenie kontrolera (jego rodzicem jest MainWindow)
    m_controller = new SimulationController(this);

    //Podłączenie przycisku do otwierania okna
    connect(ui->simulatorBtn, &QPushButton::clicked, this, &MainWindow::on_simulatorBtn_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Otwiera okno symulacji. Przy pierwszym kliknięciu tworzy SimWindow
// i ustanawia połączenia sygnałów między kontrolerem a oknem:
// - kontroler -> okno: nowe dane (per slot), statystyki
// - okno -> kontroler: import CSV, start, stop, reset (wszystko z indeksem slotu)
// Przy kolejnych kliknięciach tylko pokazuje istniejące okno.
void MainWindow::on_simulatorBtn_clicked()
{
    if (!m_simWindow){
        m_simWindow = new SimWindow(this);

        connect(m_simWindow, &QObject::destroyed, this, [this]() {
            m_simWindow = nullptr;
            qDebug() << "Okno symulatora zniszczone, a wskaźnik wyzerowany";
        });

        //sygnały kontroler <-> okno symulacji
        connect(m_controller, &SimulationController::newValueProduced, m_simWindow, &SimWindow::onNewData);
        connect(m_controller, &SimulationController::statsReady, m_simWindow, &SimWindow::onStatsReceived);
        connect(m_simWindow, &SimWindow::dataImportRequested, m_controller, &SimulationController::loadNewData);
        connect(m_simWindow, &SimWindow::startRequested, m_controller, &SimulationController::startSimulation);
        connect(m_simWindow, &SimWindow::stopRequested, m_controller, &SimulationController::stopSimulation);
        connect(m_simWindow, &SimWindow::resetRequested, m_controller, &SimulationController::resetSimulation);
    }

    m_simWindow->show();
    m_simWindow->activateWindow();
    m_simWindow->raise();
}
