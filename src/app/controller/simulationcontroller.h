#ifndef SIMULATIONCONTROLLER_H
#define SIMULATIONCONTROLLER_H
#include <QObject>
#include <QMap>
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
    explicit SimulationController(QObject * parent = nullptr);

signals:
    void newValueProduced(int slotIndex, double time, double value);
    void simulationFinished(int slotIndex);
    void statsReady(const SimulationStats& stats);

public slots:
    void loadNewData(int slotIndex, const QString &filePath);
    void startSimulation(int slotIndex);
    void stopSimulation(int slotIndex);
    void resetSimulation(int slotIndex);
    void setSimulationSpeed (int slotIndex, double multiplier);

private slots:
    void onError(const QString & message);

private:
    DataLoader * m_loader;
    QMap<int, Simulator*> m_simulators;
};

#endif // SIMULATIONCONTROLLER_H
