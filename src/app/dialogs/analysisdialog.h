#ifndef ANALYSISDIALOG_H
#define ANALYSISDIALOG_H

#include <QDialog>
#include <QVariant>
#include <QStringList>
#include <QVector>

class QComboBox;
class QLineEdit;
class QDoubleSpinBox;
class QListWidget;
class QDialogButtonBox;

class analysisdialog : public QDialog
{
    Q_OBJECT

public:
    explicit analysisdialog(const QStringList &slotLabels, QWidget *parent = nullptr);

    //wyniki do odczytania po zaakceptowaniu dialogu (exec() == QDialog::Accepted)
    QString algorithm() const;              //"kalman" lub ekf_value
    QStringList filePaths() const;           //jeden wpis na każdy dodany plik, w kolejności dodania
    QStringList sourceColumns() const;      //rownolegla do filePaths - nazwa kolumny w danym pliku
    QStringList targetColmns () const;      //rownoległa do filePaths - nazwa po połączeniu (używana tylko gdy wiecej niz jeden plik)
    QStringList featureColumns () const;    // finalne kolumny dla algorytmu, gotowe do wysłania do run_analysis.py

    int targetSlot() const;
    QVariantMap extraParams() const;        //parametry algorytmu, klucz CLI (bez "--") -> wartość

private slots:
    void onAlgorithmChanged (int index);
    void onAddFile();
    void onRemoveFile();
    void onMoveUp();
    void onMoveDown();
    void onBrowseSessionFile();
    void onAccept();

private:
    struct FileEntry {
        QString filePath;
        QString sourceColumn;
        QString targetColumn;
    };

    void updateFieldsVisibility();
    void refreshFileList();

    QComboBox *m_comboAlgorithm;

    QListWidget *m_fileListWidget;
    QVector<FileEntry> m_fileEntries;

    //parametry fizyczne EKF zaworu (widoczne tylko, gdy wybrano EKF)

    QWidget *m_ekfFieldsWidget;
    QDoubleSpinBox *m_cvMaxSpin;
    QDoubleSpinBox *m_specificGravitySpin;
    QComboBox *m_valueCharacteristicCombo;
    QDoubleSpinBox *m_rangeabilitySpin;

    //sesja (opcjonalna tylko dla EKF)
    QLineEdit *m_sessionFileEdit;
    QDoubleSpinBox *m_maxGapSecondsSpin;
    QDoubleSpinBox *m_gapResetUncertaintySpin;

    QComboBox *m_comboSlot;

    QDialogButtonBox *m_buttonBox;

};

#endif // ANALYSISDIALOG_H
