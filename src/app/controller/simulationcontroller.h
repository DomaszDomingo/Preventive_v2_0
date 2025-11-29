#ifndef SIMULATIONCONTROLLER_H
#define SIMULATIONCONTROLLER_H
#include <QObject>
#include "../dataloader/dataloader.h"
#include "../simulation/simulator.h"
#include "../simulation/strategyfactory.h"
#include <QDebug>
#include "../utils/dataanalyzer.h"


class DataLoader;
class Simulator;

class SimulationController : public QObject {

    Q_OBJECT

public:
    explicit SimulationController(QObject * parrent = nullptr);

signals:
    //Nowy sygnał, który bedzie przekazywał wartośc z symulatora
    void newValueProduced(double time, double value);
    void simulationFinished();
    void statsReady(const SimulationStats& stats);


public slots:
    void runSimulation (const QString & filePath, StrategyType strategyType);
    void loadNewData (const QString &filePath); // nowy slot do dynamicznego ladowania danych
    void startSimulation();
    void stopSimulation();
    void resetSimulation();

private slots:

    void onNewValue (double value);
    void onFinished();
    void onError(const QString & message);
    //void onDataLoaded(const QList<DataPoint>& data);

private:
    DataLoader * m_loader;
    Simulator * m_simulator = nullptr;
};

#endif // SIMULATIONCONTROLLER_H
