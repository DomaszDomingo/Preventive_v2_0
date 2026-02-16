#ifndef LIMITSDIALOG_H
#define LIMITSDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPair>
#include <QVector>

class LimitsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LimitsDialog(int slotCount, QWidget *parent = nullptr);

    void setLimits(int slotIndex, double min, double max);
    QPair<double, double> getLimits(int slotIndex) const;

signals:
    void limitsApplied(int slotIndex, double minVal, double maxVal);

private slots:
    void onSlotChanged(int comboIndex);
    void onApply();

private:
    void saveCurrentSlot();

    QComboBox *m_comboSlot;
    QDoubleSpinBox *m_spinMin;
    QDoubleSpinBox *m_spinMax;

    struct SlotLimits {
        double min = 0.0;
        double max = 0.0;
    };
    QVector<SlotLimits> m_limits;
};

#endif // LIMITSDIALOG_H
