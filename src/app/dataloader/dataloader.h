#ifndef DATALOADER_H
#define DATALOADER_H

#include <QObject>
#include <QList>
#include "../datamodel/DataPoint.h"

class DataLoader : public QObject{
    Q_OBJECT

public:
    explicit DataLoader (QObject *parent = nullptr);

signals:

    //Emitowany po pmyśnym wczytaniu i sparsowaniu danych
    //Przekazujemy przez stałą referencję, aby uniknąć zbędnej kopii.
    void dataLoaded (const QList<DataPoint> & data);

    //Emitowany w przypadku błędy (np. plik nie istnieje)

    void errorOccured (const QString & message);

public slots:

    //slot, który rozpoczyna proces ładowania. Moze być wywołany z innego wątku.
    void loadFromCSV (const QString & filePath);

};



#endif // DATALOADER_H
