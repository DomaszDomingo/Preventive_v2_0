#ifndef LIMITSDIALOG_H
#define LIMITSDIALOG_H

#include <QDialog>
#include <QCheckBox>
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
    void setLimitsVisible(int slotIndex, bool visible);
    QPair<double, double> getLimits(int slotIndex) const;

signals:
    void limitsApplied(int slotIndex, double minVal, double maxVal);
    void limitsVisibilityChanged(int slotIndex, bool visible);

private slots:
    void onSlotChanged(int comboIndex);
    void onApply();
    void onVisibilityToggled(bool checked);

private:
    void saveCurrentSlot();

    QComboBox *m_comboSlot;
    QDoubleSpinBox *m_spinMin;
    QDoubleSpinBox *m_spinMax;
    QCheckBox *m_checkVisible;

    struct SlotLimits {
        double min = 0.0;
        double max = 0.0;
        bool visible = true;
    };
    QVector<SlotLimits> m_limits;
};

#endif // LIMITSDIALOG_H
