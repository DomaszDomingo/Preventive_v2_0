#include "analysisdialog.h"

#include <QComboBox>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QListWidget>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QfileInfo>
#include <QSet>


AnalysisDialog::AnalysisDialog(const QStringList &slotLabels, QWidget *parent) : QDialog(parent)
{
    setWindowTitle (tr("Analiza Python"));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    //Wybor algorytmu

    QFormLayout *topForm = new QFormLayout();
    m_comboAlgorithm = new QComboBox(this);
    m_comboAlgorithm -> addItem(tr("Filtr Kalmana"), "kalman");
    m_comboAlgorithm -> addItem(tr("EKF Zaworu"), "ekf_valve");
    topForm->addRow(tr("Algorymtm"), m_comboAlgorithm);
    mainLayout->addLayout(topForm);

    //lista plików danych - każdy plik to jeden kanał pomiarowyc. Kolejność ma znaczenie
    //dla EKF Zaworu 1. wpis = przepływ, 2 wpis = ciśnienei, kolejne - gdy przyjdą nowe kanały)
    //to następne pozycje. Dla kalmana jest uzywany jeden wpis

    QGroupBox *filesGroup = new QGroupBox(tr("Pliki danych (kolejność ma znaczenie - dla EKF: 1. przepływ, 2. ciśnienie)"),this);
    QVBoxLayout *filesLayout = QVBoxLayout(filesGroup);

    m_fileListWidget = new QListWidget(filesGroup);
    filesLayout->addWidget(m_fileListWidget);

    QHBoxLayout *fileButtonsLayout = new QHBoxLayout();
    QPushButton *addFileBtn = new QPushButton(tr("Dodaj plik"),filesGroup);
    QPushButton *removeFileBtn = new QPushButton(tr("Usuń"),filesGroup);
    QPushButton *moveUpBtn = new QPushButton(tr("Przesuń wyżej"), filesGroup);
    QPushButton *moveDownBtn = new QPushButton(tr("Przesuń niżej"),filesGroup);
    fileButtonsLayout->addWidget(addFileBtn);
    fileButtonsLayout->addWidget(removeFileBtn);
    fileButtonsLayout->addWidget(moveUpBtn);
    fileButtonsLayout->addWidget(moveDownBtn);
    filesLayout->addLayout(fileButtonsLayout);

    mainLayout->addWidget(filesGroup);

    //Parametry fizyczne EKF Zaowu (widoczne tylko gdy wybrano EKF)
    m_ekfFieldsWidget = new QWidget(this);
    QFormLayout *ekfForm = new QFormLayout(m_ekfFieldsWidget);

    m_cvMaxSpin = new QDoubleSpinBox(m_ekfFieldsWidget);
    m_cvMaxSpin->setRange(0.1, 100000.0);
    m_cvMaxSpin->setValue(100.0);
    ekfForm->addRow(tr("Cv_max:"),m_cvMaxSpin);

    m_specificGravitySpin = new QDoubleSpinBox(m_ekfFieldsWidget);
    m_specificGravitySpin->setRange(0.1, 100.0);
    m_specificGravitySpin->setDecimals(3);
    m_specificGravitySpin->setValue(1.0);
    ekfForm->addRow(tr("Gęstość względna (SG):"),m_specificGravitySpin);

    m_valveCharacteristicCombo = new QComboBox(m_ekfFieldsWidget);
    m_valveCharacteristicCombo->addItem (tr("Liniowa"),"linear");
    m_valveCharacteristicCombo->addItem (tr("Stałoprocentowa"),"equal_percentage");
    ekfForm->addRow(tr("Charakterystyka zaworu:"),m_valveCharacteristicCombo);

    m_rangeabilitySpin = new QDoubleSpinBox(m_ekfFieldsWidget);
    m_rangeabilitySpin->setRange(1.0, 1000.0);
    m_rangeabilitySpin->setValue(50.0);
    ekfForm->addRow(tr("Zakres (dla stałoprocentowej:"),m_rangeabilitySpin);

    QHBoxLayout *sessionLayout = new QHBoxLayout();
    m_sessionFileEdit = new QLineEdit(m_ekfFieldsWidget);
    QPushButton *browseSessionBtn = new QPushButton(tr("Przeglądaj"),m_ekfFieldsWidget);
    sessionLayout->addWidget(m_sessionFileEdit);
    sessionLayout->addWidget(browseSessionBtn);
    ekfForm->addRow(tr("Plik sesji (opcjonalnie):"), sessionLayout);

    m_maxGapSecondsSpin = new QDoubleSpinBox(m_ekfFieldsWidget);
    m_maxGapSecondsSpin->setRange(1.0, 100000000.0);
    m_maxGapSecondsSpin->setValue(3600.0);
    ekfForm->addRow(tr("Maks. przerwa (s):"), m_maxGapSecondsSpin);

    m_gapResetUncertaintySpin = new QDoubleSpinBox(m_ekfFieldsWidget);
    m_gapResetUncertaintySpin->setRange(1.0, 100000000.0);
    m_gapResetUncertaintySpin->setDecimals(4);
    m_gapResetUncertaintySpin->setValue(0.1);
    ekfForm->addRow(tr("Niepewność po przerwie:"), m_gapResetUncertaintySpin);

    mainLayout->addWidget(m_ekfFieldsWidget);

    //docelowy wykres
    QFormLayout *bottomForm = new QFormLayout();
    m_comboSlot = new QComboBox(this);
    m_comboSlot->addItems(slotLabels);
    bottomForm->addRow(tr("Wykres:"), m_comboSlot);
    mainLayout->addLayout(bottomForm);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(m_buttonBox);

    connect(m_comboAlgorithm, &QComboBox::currentIndexChanged, this, &AnalysisDialog::onAlgorithmChanged);
    connect(addFileBtn, &QPushButton::clicked, this, &AnalysisDialog::onAddFile);
    connect(removeFileBtn, &QPushButton::clicked, this, &AnalysisDialog::onRemoveFile);
    connect(moveUpBtn, &QPushButton::clicked, this, &AnalysisDialog::onMoveUp);
    connect(moveDownBtn, &QPushButton::clicked, this, &AnalysisDialog::onMoveDown);
    connect(browseSessionBtn, &QPushButton::clicked, this, &AnalysisDialog::onBrowseSessionFile);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &AnalysisDialog::onAccept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    updateFieldsVisibility();



}

QString AnalysisDialog::algorithm() const
{
    return m_comboAlgorithm->currentData().toString();
}

QStringList AnalysisDialog::filePaths() const
{
    QStringList result;
    for (const FileEntry &entry : m_fileEntries)
        result << entry.filePath;
    return result;
}

QStringList AnalysisDialog::sourceColumns() const
{
    QStringList result;
    for (const FileEntry &entry : m_fileEntries)
        result << entry.sourceColumn;
    return result;
}

QStringList AnalysisDialog::targetColmuns() const
{
    QStringList result;
    for (const FileEntry &entry : m_fileEntries)
        result << entry.targetColumn;
    return result;
}

QStringList AnalysisDialog::featureColumns() const
{
    //jeden plik: run_analysis.py nie robi zadnego przemianowania - uzywa kolumny wprost
    // z pliku (source), wiec feature-columns musi byc nazwa zródlowa
    if (m_fileEntries.size() == 1)
        return QStringList() << m_fileEntries[0].sourceColumn;
    //wiecej plików: run_analysis.py laczy je i przemianowuje kolumny na "target"
    //wiec feature columns musi być lista nazw docelowych w tej samej kolejności
    return targetColumns();
}

int AnalysisDialog::targetSlot() const
{
    return m_comboSlot->currentIndex();
}

QVariantMap AnalysisDialog::extraParams() const
{
    QVariantMap params;
    if(algorithm() == "ekf_valve"){
        params["cv-max"] = m_cvMaxSpin->value();
        params["specific-gravity"] = m_specificGravitySpin->value();
        params["valve-characteristic"] = m_valveCharacteristicCombo->currentData().toString();
        params["rangeability"] = m_rangeabilitySpin->value();
        if(!m_sessionFileEdit->text().trimmed().isEmpty()){
            params["session-file"] = m_sessionFileEdit->text().trimmed();
            params["max-gap-seconds"] = m_maxGapSecondsSpin->value();
            params["gap-reset-uncertainty"] = m_gapResetUncertaintySpin->value();
            }
    }

    return params;
}




void AnalysisDialog::onAlgorithmChanged(int index)
{
    Q_UNUSED(index);
    updateFieldsVisibility();
}

void AnalysisDialog::onAddFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Wybierz plik danych"), QString(), tr("CSV Files (*.csv);;All Files(*)"));
    if (fileName.isEmpty())
        return;

    bool ok = false;
    QString sourceColumn = QInputDialog::getText(this, tr("Kolumna źródłowa"),
    tr("Nazwa kolumny w tym pliku (po normalizacji, małymi literami), np \"wartość\":"),
    QLineEdit::Normal, "wartość", &ok);

    if(!ok || sourceColumn.trimmed().isEmpty())
        return;

    QString targetColumn = QInputDialog::getText(this, tr("Nazwa docelowa"),
    tr("np.\"przepływ\", \"ciśnienie\"). Potrzebne tylko gdy dodajesz wiecej niż jeden plik:"),
        QLineEdit::Normal,QString(), &ok);
    if (!ok)
        return;

    FileEntry entry;
    entry.filePath = fileName;
    entry.sourceColumn = sourceColumn.trimmed();
    entry.targetColumn = targetColumn.trimmed();
    m_fileEntries.append(entry);
    refreshFile();


}

void AnalysisDialog::onRemoveFile()
{
    int row = m_fileListWidget->currentRow();
    if (row < 0 || row >= m_fileEntries.size());
        return;
    m_fileEntries.removeAt(row);
    refreshFileList();

}

void AnalysisDialog::onMoveUp()
{
    int row = m_fileListWidget->currentRow();
    if (row <=0 || row >= m_fileEntries.size())
        return;
    m_fileEntries.swapItemsAt(row, row - 1);
    refreshFileList();
    m_fileListWidget->setCurrentRow(row - 1);

}

void AnalysisDialog::onMoveDown()
{
    int row = m_fileListWidget->currentRow();
    if (row < 0 || row >= m_fileEntries.size() - 1)
        return;
    m_fileEntries.swapItemsAt(row, row + 1);
    refreshFileList();
    m_fileListWidget->setCurrentRow(row + 1);
}

void AnalysisDialog::onBrowseSessionFile()
{
    //getSaveFileName, bo plik sesji moze jeszcze neie istniec (pierwsze uzycie)
    //a rownie dobrze moze wskazwyać na juz istniejący plik do wznowienia

    QString fileName = QFileDialog::getSaveFileName(this, tr("Plik sesji EKF"), QString(), tr("JSON (*.json);;All files(*)"));
    if (!fileName.isEmpty())
        m_sessionFileEdit->setText(fileName);
}

void AnalysisDialog::onAccept()
{
    if(m_fileEntries.isEmpty()){
        QMessageBox::warning(this, tr("Analiza Python"), tr("Dodaj co najmniej jeden plik."));
        return;
    }

    bool isEKF = (algorithm() == "ekf_valve");

    if (!isEKF && m_fileEntries.size() != 1) {
        QMessageBox::warning(this, tr("Analiza Python)"),
                             tr("Filtr Kalmana wymaga tylko jednego pliku/kolumny. Usuń pozostałe wpisy"));
        return;
    }

    if (!isEKF && m_fileEntries.size() < 2) {
        QMessageBox::warning(this, tr("Analiza Python)"),
                             tr("EKF Zaworu wymaga co najmniej dwoch plików/kolumn (przepływ, ciśnienie)."));
        return;
    }

    if(m_fileEntries.size() > 1){
        QSet<QString> seenTargets;
        for (const FileEntry &entry : m_fileEntries){
            if(entry.targetColumn.isEmpty()){
                QMessageBox::warning(this, tr("Analiza Python"),
                                     tr("Przy wiecej niż jednym pliku kazdy wpis musi mieć nazwę docelową."));
                return;
            }
            if(seenTargets.contains(entry.targetColumn)){
                QMessageBox::warning(this, tr("Analiza Python"),
                                     tr("Nazwy docelowe muszą się różnić (powtórzona nazwa: \%1\").").arg(entry.targetColumn));
                return;
            }
            seenTargets.insert((entry.targetColumn));

        }
    }

    accept();
}

void AnalysisDialog::updateFieldsVisibility()
{
    bool isEkf = (m_comboAlgorithm->currentData().toString() == "ekf_valve");
    m_ekfFieldsWidget->setVisible(isEkf);
}

void AnalysisDialog::refreshFileList()
{
    m_fileListWidget->clear();
    for (int i = 0; i < m_fileEntries.size(); i++){
        const FileEntry &entry = m_fileEntries[i];
        QString label = tr("%1. %2 (kolumna: %3%4)")
                            .arg(i + 1)
                            .arg(QFileInfo(entry.filePath).fileName())
                            .arg(entry.sourceColumn)
                            .arg(entry.targetColumn.isEmpty() ? QString() : tr(" -> %1").arg(entry.targetColumn));
        m_fileListWidget->addItem(label);
    }
}









