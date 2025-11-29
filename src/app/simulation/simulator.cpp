#include "simulator.h"
#include <QDebug>

//ustawiamy częstotliwość odswiezania symulacji (w Hz) 60 Hz to płynna aktualizacja

constexpr int SIMULATION_TICK_RATE_HZ = 60;

Simulator::Simulator(std::unique_ptr<ISimulationStrategy> strategy, QObject * parent)
    : QObject(parent), m_strategy(std::move(strategy)){
    connect (&m_timer, &QTimer::timeout, this, &Simulator::onTick);
}

Simulator::~Simulator (){
    qDebug() << "Symulator zniszczony.";
}

void Simulator::start()
{
    if(m_data.isEmpty() || !m_strategy){
        qWarning() << "Nie można uruchomić symulacji: brak danych lub nie wybrana strategia.";
        return;
    }
    m_elapsedTimer.start();
    m_timer.start(1000/SIMULATION_TICK_RATE_HZ);
    qInfo() << "Symulacja uruchomiona.";
}

void Simulator::stop()
{
    if(m_timer.isActive()){
        m_timer.stop();;
        emit simulationFinished();
        qInfo() << "Symulacja zatrzymana.";
    }
}

bool Simulator::isRunning() const
{
    return m_timer.isActive();
}



void Simulator::setData(const QList<DataPoint> &data)
{
    m_data = data;
    if(!m_data.isEmpty()){
        m_simulationDuration = m_data.last().timestamp;
    } else {
        m_simulationDuration = 0;
    }

    qInfo() << "Zbiór danych symulacji. Czas trwania:" << m_simulationDuration << "ms";
}

void Simulator::reset()
{
    stop();

    if(!m_data.isEmpty()){

        //emit (time = 0, value = first value)

        emit valueChanged(0.0, m_data.first().value);

        qInfo() << "Symulacja zresetowana.";

    }
}

void Simulator::onTick()
{
    qint64 elapsed = m_elapsedTimer.elapsed();
    if (elapsed > m_simulationDuration){
        stop ();
        if(!m_data.isEmpty()){
        //Upewniamy sie, że na końcu emitujemy ostanią wartość
            emit valueChanged((double)m_simulationDuration, m_data.last().value);
        }
        return;
        }
    double currentValue = m_strategy->process(elapsed, m_data);
        emit valueChanged((double)elapsed,currentValue);
}




