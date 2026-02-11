#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "simwindow.h"
#include <QDebug>
#include "controller/simulationcontroller.h"
#include "simulation/StrategyFactory.h"



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Utworzenie kontrolera (jego rodzicem jest MainWindow)
    m_controller = new SimulationController (this);

    //Podłączenie przycisku do otwierania okna
    connect (ui->simulatorBtn, &QPushButton::clicked, this, &MainWindow::on_simulatorBtn_clicked);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_simulatorBtn_clicked()
{
    //sprawdzenie czy okno jescze nie istnieje
    if (!m_simWindow){
        //jeśli nie to tworzymy
        m_simWindow = new SimWindow(this); //this jako rodzic

        //Musimy obsłużyć sytuację, gdy użytkownik zamknie okno
        //Przyciskiem 'X'. Okno zostanie zniszczone (bo ma rodzica),
        //ale nasz wskaźnik "m_simulatorWindow" nadal by na nie wskazywał
        //Łączymy sygnał destroyed okna z lambdą, która wyzeruje wskaźnik

        connect (m_simWindow, &QObject::destroyed, this, [this]() {
            m_simWindow = nullptr;
            qDebug () << "Okno symulatora zniszczone, a wskaźnik wyzerowany";
        });

        //Połaczenie sygnału z kontrolera ze slotem w oknie wykresu
        connect(m_controller, &SimulationController::newValueProduced, m_simWindow, &SimWindow::onNewData);
        connect(m_controller, &SimulationController::statsReady, m_simWindow, &SimWindow::onStatsReceived);
        connect(m_simWindow, &SimWindow::dataImportRequested, m_controller, &SimulationController::loadNewData);
        connect(m_simWindow, &SimWindow::startRequested, m_controller, &SimulationController::startSimulation);
        connect(m_simWindow, &SimWindow::stopRequested, m_controller, &SimulationController::stopSimulation);
        connect(m_simWindow, &SimWindow::resetRequested, m_controller, &SimulationController::resetSimulation);
    }


    m_controller->runSimulation("", StrategyType::Linear);
    //2. niezależnie czy okno było, czy dopiero zostało stworzone
    m_simWindow->show();
    m_simWindow->activateWindow();
    m_simWindow->raise();


}
