#include "simwindow.h"
#include "ui_simwindow.h"
#include "qcustomplot.h"
#include <QDebug> // Do logowania błędów
#include <QFileDialog>
#include <QMessageBox>



SimWindow::SimWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SimWindow)
   {
    ui->setupUi(this);

    m_infoWindow = new InfoWindow(this);
    
    //ukrycie starego pliku widget
    if(ui->plotWidget){
        ui->plotWidget->setVisible(false);
    }
    
    //inicjalizacja siatki wykresow
    setupLayout();

    connect (ui->closeSimulatorBtn, &QPushButton::clicked, this, &SimWindow::close);
    connect (ui->importCsvBtn, &QPushButton::clicked, this, &SimWindow::handleImportCsv);

    connect(ui->btnStart, &QPushButton::clicked, this, [this](){
        ui->btnStart->setEnabled(false);
        ui->btnStop->setEnabled(true);
        emit startRequested();

    });
    connect(ui->btnStop, &QPushButton::clicked, this, [this](){
        ui->btnStart->setEnabled(true);
        ui->btnStop->setEnabled(false);
        emit stopRequested();
    });
    connect(ui->btnReset, &QPushButton::clicked, this, [this](){
        // Resetujemy wszystkie aktywne wykresy
        for(auto slot : m_chartSlots) {
            slot->reset(); // Przywraca stan "pusty" (z przyciskiem Dodaj)
            // Lub jeśli chcesz tylko wyczyścić dane, ale zostawić wykres:
            //slot->displayChart(slot->getTrendId(), "Wykres");
        }

        ui->btnStart->setEnabled(true);
        ui->btnStop->setEnabled(false);
        emit resetRequested();
    });

    ui->btnStart->setEnabled(true);
    ui->btnStop->setEnabled(false);


}

SimWindow::~SimWindow()
{
    delete ui;
}

void SimWindow::setupLayout()
{
    // Znajdujemy miejsce, gdzie wstawić naszą siatkę.
    // Najlepiej wstawić ją w miejsce, gdzie był plotWidget.
    // Pobieramy layout rodzica plotWidgeta (czyli główny layout okna)

    // Tworzymy nowy layout dla wykresów, jeśli jeszcze nie istnieje w odpowiednim miejscu
    // Dla uproszczenia: dodamy nowy layout do głównego layoutu okna lub stworzymy go dynamicznie.

    // Zakładając, że w ui jest jakiś główny layout (np. wertykalny), ale jeśli używasz pozycjonowania absolutnego w .ui,
    // musimy zrobić to ręcznie.

    // Stwórzmy kontener na wykresy, który zajmie miejsce starego plotWidget

    QWidget * chartsContainer = new QWidget(this);
    //Ustawienie geometrii
    chartsContainer->setGeometry(39,29,600,350);

    QGridLayout * gridLayout = new QGridLayout(chartsContainer);
    gridLayout->setContentsMargins(0,0,0,0);

    m_chartSlots.clear();

    //Tworzenie 2 slotow obok siebie
    for (int i = 0; i < 2; ++i){
        ChartSlot *slot = new ChartSlot (i,this);

        // Łaczymy sygnały prosby o dodanie wykresu
        connect (slot, &ChartSlot::importRequested, this, &SimWindow::onSlotImportRequested);

        m_chartSlots.append(slot);
        gridLayout->addWidget(slot, 0 ,i);
    }

    chartsContainer->show();
}

void SimWindow::onNewData(double time, double value)
{
    double timeInSeconds = time / 1000.0;

    //przekazujemy dane do wszystkich aktywnych slotów
    for (auto slot : m_chartSlots){
        slot->addDataPoint(timeInSeconds, value);
    }

}

void SimWindow::handleImportCsv()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Importuj Dane CSV"), "", tr("CSV Files(*.csv);;AllFiles(*)")); // otwórz okno dialogowe

    if(fileName.isEmpty()) //jezeli uzytkownik anuluje
        return;

    for (auto slot : m_chartSlots){
        //czyścimy dane, ale nie zamykamy wykresu ( nie robimy reset() do przycisku)
        //Trzeba dodać metodę clearData() do chartSlot lub uzyc publicznego dostępu
        // w kodzie w ChartSlot::displayChart jest czyszczenie wiec mozna uzyć
        if(!slot->isEmpty()){
            slot->displayChart(1, "Symulacja temperatury");
        }
    }
    emit dataImportRequested(fileName); // wyemituj sygnał do controllera
}


void SimWindow::onStatsReceived(const SimulationStats &stats)
{
    if (!m_infoWindow) return;

    m_infoWindow->updateStats(stats);
    m_infoWindow->show();
    m_infoWindow->raise();
    m_infoWindow->activateWindow();

}

void SimWindow::onSlotImportRequested(int slotIndex)
{
    // Użytkownik kliknął "Dodaj Wykres" w konkretnym slocie.
    // Tutaj decydujemy co tam wyświetlić.

    // Ponieważ na razie mamy tylko jedną symulację (jeden strumień danych),
    // po prostu aktywujemy ten slot i podpisujemy go.

    if (slotIndex >= 0 && slotIndex < m_chartSlots.size()){
        ChartSlot* slot = m_chartSlots[slotIndex];
        slot->displayChart(1, "Symulacja temp");

    }
}

























