#include "pythonanalysisrunner.h"
#include <QDebug>

namespace{
//Interpreter z virtualenv Preventive_pl - ma zainstalowane pandas/numpy
//zaszyte na sztywno do czasu, az projekt dostanie osobna konfigurację ścieżek
const QString PYTHON_EXECUTABLE =
    "C:/Users/DomoVM/Documents/GitHub/Preventive_v2_0/Preventive_ML/venv/Scripts/python.exe";

const QString RUN_ANALYSIS_SCRIPT =
    "C:/Users/DomoVM/Documents/GitHub/Preventive_v2_0/Preventive_ML/run_analysis.py";
}


PythonAnalysisRunner::PythonAnalysisRunner(QObject *parent) : QObject (parent)
{
    m_process = new QProcess(this);
    connect (m_process, &QProcess::finished, this, &PythonAnalysisRunner::onProcessFinished);
    connect (m_process, &QProcess::errorOccurred, this , &PythonAnalysisRunner::onProcessErrorOccured);
}

void PythonAnalysisRunner::runAnalysis(const QString &algorithm, const QString &dataFilePath, const QStringList &featureColumns, const QString &targetColumn, const QVariantMap &extraParams)
{
    if (m_process->state() != QProcess::NotRunning){
        qWarning() << "PythonAnalysisRunner: poprzednia analiza jeszcze trwa, nowe zadanie zingorowane.";
        return;
    }

    m_currentAlgorithm = algorithm;

    QStringList args;

    args << RUN_ANALYSIS_SCRIPT << "--file" << dataFilePath << "--algorithm" << algorithm << "--feature-columns" << featureColumns;

    if(!targetColumn.isEmpty()){
        args << "--target-column" << targetColumn;
    }

    //Dowolne dodatkowe parametry algorytmu (np. klucz "process-noise" -> --process-noise).
    for (auto it = extraParams.constBegin(); it != extraParams.constEnd();++it){
        args  << ("--" + it.key()) << it.value().toString();
    }

    m_process->start(PYTHON_EXECUTABLE,args);
}

void PythonAnalysisRunner::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);

    QByteArray output = m_process->readAllStandardOutput();
    AnalysisResult result = AnalysisResult::fromJson(output);
    emit analysisFinished(result);
}

void PythonAnalysisRunner::onProcessErrorOccured(QProcess::ProcessError error)
{
    Q_UNUSED(error);
    emit analysisFinished(AnalysisResult::error(m_currentAlgorithm, "Nie udało się uruchomic procesu Python: " + m_process->errorString()));
}


