#include "simulationcontroller.h"
#include "../simulation/simulator.h"
#include "../simulation/StrategyFactory.h"
#include "../dataloader/dataloader.h"
#include <QDebug> //

SimulationController::SimulationController(QObject *parent) : QObject(parent)  {
    //DataLoader jest stałym komponentem kontrolera
    m_loader = new DataLoader(this);

    connect(m_loader, &DataLoader::errorOccured, this, &SimulationController::onError);

}

void SimulationController::runSimulation(const QString &filePath, StrategyType strategyType)
{
    //jeśli symulator juz istneije (z poprzedniego uruchomienia), usuwamy go.
    //Dzięki temu mozemy wielokrotnie uruchamiać symulację z różnymi strategiami
    if(m_simulator){
        m_simulator->deleteLater();
    }

    //1. Tworzymy nową strategię za pomoca fabryki
    auto strategy = StrategyFactory::create(strategyType);
    if(!strategy){
        onError ("Niepowodzenie w tworzeniu strategii symulacji.");
        return;
    }
    //2. Tworzymy nowy symulator z wybraną stragetią
    m_simulator = new Simulator(std::move(strategy), this);

    //3.Łączymy sygnały symulatora z naszymi slotami
    connect (m_simulator, &Simulator::valueChanged, this, &SimulationController::newValueProduced);
    connect (m_simulator, &Simulator::simulationFinished, this, &SimulationController::simulationFinished);

    //połaczenie loadera z symulatorem
    connect(m_loader, &DataLoader::dataLoaded, this, [this](const QList<DataPoint>& data){
        if (m_simulator) {
            m_simulator->setData(data); // Krok 1: Ustaw dane
            m_simulator->start();       // Krok 2: Ręcznie uruchom symulację
        }
    });



    m_loader->loadFromCSV(filePath);
}

void SimulationController::onNewValue(double value)
{
   //Tutaj UI (np. Wykres) by się aktualizowało
    qDebug().nospace() << "Obecna symulowana wartość: " << qSetRealNumberPrecision(2) << value;
}

void SimulationController::onFinished()
{
    qInfo() << "-- Symulacja zakończona --";
}

void SimulationController::onError(const QString &message)
{
    qCritical() << "Błąd: " << message;
}
