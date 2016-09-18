#pragma once

#include <QtCore>

/// Event Filter to prevent mouse wheel events being accepted - use
/// on SpinBoxes.

class MouseWheelDisabler : public QObject
{
    Q_OBJECT;
public:
    
    MouseWheelDisabler(QObject* parent=nullptr) : QObject(parent) {}
    
protected:
    virtual bool eventFilter(QObject* obj, QEvent* event) override;
};
