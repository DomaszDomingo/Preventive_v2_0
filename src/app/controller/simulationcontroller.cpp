#include "simulationcontroller.h"
#include "../simulation/simulator.h"
#include "../simulation/StrategyFactory.h"
#include "../dataloader/dataloader.h"
#include <QDebug> //
#include "../simwindow.h"

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
    connect(m_simulator, &Simulator::valueChanged, this, [this](double time, double value){
        emit newValueProduced(time, value);
    });
    connect (m_simulator, &Simulator::simulationFinished, this, &SimulationController::simulationFinished);


    if(!filePath.isEmpty())
        m_loader->loadFromCSV(filePath);
}

void SimulationController::loadNewData(const QString &filePath)
{
    //zatrzymaj istniejący symulator
    if (m_simulator) m_simulator->stop();

    disconnect (m_loader, &DataLoader::dataLoaded, nullptr, nullptr);
    connect(m_loader, &DataLoader::dataLoaded, this, [this, filePath](const QList<DataPoint> &data){
    if(m_simulator){
        m_simulator->setData(data);
        //m_simulator->start();
    }

    //kontroler nie wie jak obliczac wiec pyta analyzer o zrobienie tego
        SimulationStats stats = DataAnalyzer::analyze(data,filePath);
        emit statsReady(stats);
    });
    // Ponownie użyj istniejącego modułu ładującego, aby załadować nowy plik.
    // Ponieważ już połączyłeś m_loader::dataLoaded z lambdą w konstruktorze/runSimulation,
    // wywołanie tej funkcji automatycznie spowoduje, że symulator zaktualizuje swoje dane i uruchomi się ponownie!
    m_loader->loadFromCSV(filePath);
}

void SimulationController::startSimulation()
{
    if(m_simulator && !m_simulator->isRunning())
        m_simulator->start();;
}

void SimulationController::stopSimulation()
{
    if(m_simulator && m_simulator->isRunning())
        m_simulator->stop();
}

void SimulationController::resetSimulation()
{
    if(m_simulator)
        m_simulator->reset();
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
