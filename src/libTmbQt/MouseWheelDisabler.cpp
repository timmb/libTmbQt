#include "MouseWheelDisabler.h"

bool MouseWheelDisabler::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Wheel)
    {
        return true;
    }
    return QObject::eventFilter(obj, event);
}
