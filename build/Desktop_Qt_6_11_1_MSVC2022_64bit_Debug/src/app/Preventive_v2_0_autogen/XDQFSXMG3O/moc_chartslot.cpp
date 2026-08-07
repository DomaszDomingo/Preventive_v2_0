/****************************************************************************
** Meta object code from reading C++ file 'chartslot.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../src/app/components/chartslot.h"
#include <QtGui/qtextcursor.h>
#include <QtGui/qscreen.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'chartslot.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN9ChartSlotE_t {};
} // unnamed namespace

template <> constexpr inline auto ChartSlot::qt_create_metaobjectdata<qt_meta_tag_ZN9ChartSlotE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ChartSlot",
        "addChartRequested",
        "",
        "slotIndex",
        "csvLoadRequested",
        "startRequested",
        "stopRequested",
        "resetRequested",
        "speedChanged",
        "multiplier"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'addChartRequested'
        QtMocHelpers::SignalData<void(int)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Signal 'csvLoadRequested'
        QtMocHelpers::SignalData<void(int)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Signal 'startRequested'
        QtMocHelpers::SignalData<void(int)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Signal 'stopRequested'
        QtMocHelpers::SignalData<void(int)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Signal 'resetRequested'
        QtMocHelpers::SignalData<void(int)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Signal 'speedChanged'
        QtMocHelpers::SignalData<void(int, double)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 }, { QMetaType::Double, 9 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ChartSlot, qt_meta_tag_ZN9ChartSlotE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ChartSlot::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9ChartSlotE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9ChartSlotE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN9ChartSlotE_t>.metaTypes,
    nullptr
} };

void ChartSlot::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ChartSlot *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->addChartRequested((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->csvLoadRequested((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->startRequested((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->stopRequested((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 4: _t->resetRequested((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 5: _t->speedChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<double>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ChartSlot::*)(int )>(_a, &ChartSlot::addChartRequested, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ChartSlot::*)(int )>(_a, &ChartSlot::csvLoadRequested, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (ChartSlot::*)(int )>(_a, &ChartSlot::startRequested, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (ChartSlot::*)(int )>(_a, &ChartSlot::stopRequested, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (ChartSlot::*)(int )>(_a, &ChartSlot::resetRequested, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (ChartSlot::*)(int , double )>(_a, &ChartSlot::speedChanged, 5))
            return;
    }
}

const QMetaObject *ChartSlot::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ChartSlot::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9ChartSlotE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int ChartSlot::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void ChartSlot::addChartRequested(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void ChartSlot::csvLoadRequested(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void ChartSlot::startRequested(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void ChartSlot::stopRequested(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void ChartSlot::resetRequested(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void ChartSlot::speedChanged(int _t1, double _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1, _t2);
}
QT_WARNING_POP
