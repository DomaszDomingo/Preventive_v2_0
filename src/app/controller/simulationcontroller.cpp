#include "simulationcontroller.h"
#include "../simulation/simulator.h"
#include "../simulation/StrategyFactory.h"
#include "../dataloader/dataloader.h"
#include <QDebug>

// Konstruktor kontrolera. Tworzy współdzielony DataLoader
// i podłącza jego sygnał błędów do obsługi.
SimulationController::SimulationController(QObject *parent) : QObject(parent)  {
    m_loader = new DataLoader(this);
    connect(m_loader, &DataLoader::errorOccured, this, &SimulationController::onError);
}

// Ładuje dane CSV do symulatora przypisanego do danego slotu.
// Jeśli symulator dla tego slotu jeszcze nie istnieje, tworzy nową instancję
// z domyślną strategią (Linear) i podłącza jego sygnały z przechwyceniem slotIndex,
// dzięki czemu każdy slot emituje dane niezależnie.
// Po utworzeniu/znalezieniu symulatora zatrzymuje go, parsuje plik CSV
// przez DataLoader i po załadowaniu ustawia dane w symulatorze.
void SimulationController::loadNewData(int slotIndex, const QString &filePath)
{
    if (!m_simulators.contains(slotIndex)) {
        auto strategy = StrategyFactory::create(StrategyType::Linear);
        if (!strategy) {
            onError("Niepowodzenie w tworzeniu strategii symulacji.");
            return;
        }
        Simulator *sim = new Simulator(std::move(strategy), this);

        //podlaczamy sygnaly z przechwyceniem slotIndex
        connect(sim, &Simulator::valueChanged, this, [this, slotIndex](double time, double value){
            emit newValueProduced(slotIndex, time, value);
        });
        connect(sim, &Simulator::simulationFinished, this, [this, slotIndex](){
            emit simulationFinished(slotIndex);
        });

        m_simulators[slotIndex] = sim;
    }

    Simulator *sim = m_simulators[slotIndex];
    sim->stop();

    //ladujemy dane - trzeba odlaczyc poprzednie polaczenie i podlaczyc nowe z kontekstem tego slotu
    disconnect(m_loader, &DataLoader::dataLoaded, nullptr, nullptr);
    connect(m_loader, &DataLoader::dataLoaded, this, [this, sim, filePath](const QList<DataPoint> &data){
        sim->setData(data);

        SimulationStats stats = DataAnalyzer::analyze(data, filePath);
        emit statsReady(stats);

        //odlaczamy po uzyciu zeby nie nakladac wielokrotnych polaczen
        disconnect(m_loader, &DataLoader::dataLoaded, nullptr, nullptr);
    });

    m_loader->loadFromCSV(filePath);
}

// Uruchamia symulację dla konkretnego slotu.
// Sprawdza czy symulator istnieje i czy nie jest już uruchomiony.
void SimulationController::startSimulation(int slotIndex)
{
    if (m_simulators.contains(slotIndex)) {
        Simulator *sim = m_simulators[slotIndex];
        if (!sim->isRunning())
            sim->start();
    }
}

// Zatrzymuje symulację dla konkretnego slotu (pauzuje timer).
void SimulationController::stopSimulation(int slotIndex)
{
    if (m_simulators.contains(slotIndex)) {
        Simulator *sim = m_simulators[slotIndex];
        if (sim->isRunning())
            sim->stop();
    }
}

// Resetuje symulację dla konkretnego slotu - zatrzymuje timer
// i ustawia czas symulacji z powrotem na początek.
void SimulationController::resetSimulation(int slotIndex)
{
    if (m_simulators.contains(slotIndex)) {
        m_simulators[slotIndex]->reset();
    }
}

void SimulationController::setSimulationSpeed(int slotIndex, double multiplier)
{
    if(m_simulators.contains(slotIndex))
        m_simulators[slotIndex]->setSpeedMultipier(multiplier);
}

void SimulationController::onError(const QString &message)
{
    qCritical() << "Błąd: " << message;
}
