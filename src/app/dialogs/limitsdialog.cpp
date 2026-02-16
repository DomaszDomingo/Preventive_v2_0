#include "limitsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>

LimitsDialog::LimitsDialog(int slotCount, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Dodaj limity"));
    resize(300, 220);

    m_limits.resize(slotCount);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Rozwijana lista slotów
    m_comboSlot = new QComboBox(this);
    for (int i = 0; i < slotCount; ++i) {
        m_comboSlot->addItem(tr("Slot %1").arg(i + 1));
    }
    mainLayout->addWidget(m_comboSlot);

    // Pola min/max
    QFormLayout *form = new QFormLayout();

    m_spinMin = new QDoubleSpinBox(this);
    m_spinMin->setRange(-1e9, 1e9);
    m_spinMin->setDecimals(4);

    m_spinMax = new QDoubleSpinBox(this);
    m_spinMax->setRange(-1e9, 1e9);
    m_spinMax->setDecimals(4);

    form->addRow(tr("Min:"), m_spinMin);
    form->addRow(tr("Max:"), m_spinMax);
    mainLayout->addLayout(form);

    // Checkbox widoczności
    m_checkVisible = new QCheckBox(tr("Wyświetlaj limity"), this);
    m_checkVisible->setChecked(true);
    mainLayout->addWidget(m_checkVisible);
    mainLayout->addStretch();

    // Przyciski
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnApply = new QPushButton(tr("Zastosuj"), this);
    QPushButton *btnCancel = new QPushButton(tr("Anuluj"), this);

    btnLayout->addStretch();
    btnLayout->addWidget(btnApply);
    btnLayout->addWidget(btnCancel);
    mainLayout->addLayout(btnLayout);

    connect(m_comboSlot, &QComboBox::currentIndexChanged, this, &LimitsDialog::onSlotChanged);
    connect(m_checkVisible, &QCheckBox::toggled, this, &LimitsDialog::onVisibilityToggled);
    connect(btnApply, &QPushButton::clicked, this, &LimitsDialog::onApply);
    connect(btnCancel, &QPushButton::clicked, this, &LimitsDialog::reject);
}

void LimitsDialog::setLimits(int slotIndex, double min, double max)
{
    if (slotIndex >= 0 && slotIndex < m_limits.size()) {
        m_limits[slotIndex].min = min;
        m_limits[slotIndex].max = max;

        if (m_comboSlot->currentIndex() == slotIndex) {
            m_spinMin->setValue(min);
            m_spinMax->setValue(max);
        }
    }
}

void LimitsDialog::setLimitsVisible(int slotIndex, bool visible)
{
    if (slotIndex >= 0 && slotIndex < m_limits.size()) {
        m_limits[slotIndex].visible = visible;

        if (m_comboSlot->currentIndex() == slotIndex) {
            m_checkVisible->setChecked(visible);
        }
    }
}

QPair<double, double> LimitsDialog::getLimits(int slotIndex) const
{
    if (slotIndex >= 0 && slotIndex < m_limits.size()) {
        return {m_limits[slotIndex].min, m_limits[slotIndex].max};
    }
    return {0.0, 0.0};
}

void LimitsDialog::saveCurrentSlot()
{
    int idx = m_comboSlot->currentIndex();
    if (idx >= 0 && idx < m_limits.size()) {
        m_limits[idx].min = m_spinMin->value();
        m_limits[idx].max = m_spinMax->value();
        m_limits[idx].visible = m_checkVisible->isChecked();
    }
}

void LimitsDialog::onSlotChanged(int comboIndex)
{
    saveCurrentSlot();

    if (comboIndex >= 0 && comboIndex < m_limits.size()) {
        m_spinMin->setValue(m_limits[comboIndex].min);
        m_spinMax->setValue(m_limits[comboIndex].max);
        m_checkVisible->setChecked(m_limits[comboIndex].visible);
    }
}

void LimitsDialog::onVisibilityToggled(bool checked)
{
    int idx = m_comboSlot->currentIndex();
    if (idx >= 0 && idx < m_limits.size()) {
        m_limits[idx].visible = checked;
        emit limitsVisibilityChanged(idx, checked);
    }
}

void LimitsDialog::onApply()
{
    saveCurrentSlot();

    for (int i = 0; i < m_limits.size(); ++i) {
        emit limitsApplied(i, m_limits[i].min, m_limits[i].max);
        emit limitsVisibilityChanged(i, m_limits[i].visible);
    }
    accept();
}
