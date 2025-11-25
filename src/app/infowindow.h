#ifndef INFOWINDOW_H
#define INFOWINDOW_H

#include <QDialog>
#include <QLabel>
#include "utils/dataanalyzer.h"

class InfoWindow : public QDialog
{
    Q_OBJECT
public:

    explicit InfoWindow(QWidget * parent = nullptr);
    void updateStats(const SimulationStats & stats);

private:
    QLabel *m_lblFile;
    QLabel *m_lblCount;
    QLabel *m_lblDuration;
    QLabel *m_lblRange;
    QLabel *m_lblAvg;
    QPushButton *m_btnClose;
};

#endif // INFOWINDOW_H
