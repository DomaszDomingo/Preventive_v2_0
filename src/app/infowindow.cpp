#include "infowindow.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QFileInfo>

InfoWindow::InfoWindow(QWidget *parent) : QDialog(parent){
    setWindowTitle("Informacje o danych");
    resize (350,250);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout();

    m_lblFile = new QLabel("-");
    m_lblFile->setWordWrap(true);
    m_lblCount = new QLabel("-");
    m_lblDuration = new QLabel("-");
    m_lblDuration = new QLabel("-");
    m_lblRange = new QLabel("-");
    m_lblAvg = new QLabel("-");

    formLayout->addRow("Nazwa pliku:", m_lblFile);
    formLayout->addRow("Liczba próbek:", m_lblCount);
    formLayout->addRow("Czas trwania:", m_lblDuration);
    formLayout->addRow("Zakres (Min/Max):", m_lblRange);
    formLayout->addRow("Średnia:",m_lblAvg);

    m_btnClose = new QPushButton("Zamknij", this);
    connect(m_btnClose, &QPushButton::clicked, this, &InfoWindow::close);

    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();
    mainLayout->addWidget(m_btnClose);
}

void InfoWindow::updateStats(const SimulationStats &stats){
    QFileInfo fi(stats.fileName);
    m_lblFile->setText(fi.fileName());
    m_lblCount->setText(QString::number(stats.count));

    double durationSec = (stats.endTime - stats.startTime) / 1000.0;
    m_lblDuration->setText(QString::number(durationSec, 'f', 2) + " s");

    m_lblRange->setText(QString("[%1; %2]")
                            .arg(stats.minValue, 0, 'f',2)
                            .arg(stats.maxValue, 0, 'f', 2));

    m_lblAvg->setText(QString::number(stats.avgValue, 'f',2));
}
