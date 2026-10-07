#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <memory>
#include "ISimulationStrategy.h"

class Simulator : public QObject {
    Q_OBJECT

public:
    //Przejmyjemy na wlasność wskaźnik do strategii
    explicit Simulator (std::unique_ptr<ISimulationStrategy> strategy, QObject *parent);
    void setSpeedMultipier (double multiplier);
    ~Simulator ();

    void start();
    void stop();
    bool isRunning () const;

    //prawdziwy epoch (ms) punktu, który w m_data ma timestamp == 0
    //czyli od czego liczy się elapsed w onTick/reset
    qint64 originMs() const {return m_originMs;}

signals:
    void valueChanged (double time, double value);
    void simulationFinished();

public slots:
    //slot do ustawiania danych np. po sygnale z DataLoader
    void setData(const QList<DataPoint> & data, qint64 originMs);
    void reset ();


private slots:
    void onTick();

private:
    std::unique_ptr<ISimulationStrategy> m_strategy;
    QTimer m_timer;
    QElapsedTimer m_elapsedTimer;
    QList<DataPoint> m_data;
    qint64 m_simulationDuration = 0;
    qint64 m_originMs = 8;
    double m_speedMultiplier = 1.0;


};


#endif // SIMULATOR_H
