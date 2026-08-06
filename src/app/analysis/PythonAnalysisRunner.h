#ifndef PYTHONANALYSIS_H
#define PYTHONANALYSIS_H

#include <QObject>
#include <QProcess>
#include <QStringList>
#include <QVariantMap>
#include "../datamodel/analysisresult.h"

class PythonAnalysisRunner : public QObject
{
    Q_OBJECT

public:
    explicit PythonAnalysisRunner (QObject *parent = nullptr);

    //Uruchamia run_analysis.py asynchronicznie. Wynik przechodzi przez sygnał
    //AnalysisFinished, gdy proces sie zakonczy (sukces lub błąd)

    void runAnalysis (const QString &algorithm,
                     const QString &dataFilePath,
                     const QStringList &featureColumns,
                     const QString &targetColumn = QString(),
                     const QVariantMap &extraParams = QVariantMap());

signals:
    void analysisFinished(const AnalysisResult &result);

private slots:
    void onProcessFinished (int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessErrorOccured (QProcess::ProcessError error);

private:
    QProcess *m_process;
    QString m_currentAlgorithm; // potrzebne do AnalysisResult::error(), gdy proces w ogole się nie uruchomi
};


#endif // PYTHONANALYSIS_H
