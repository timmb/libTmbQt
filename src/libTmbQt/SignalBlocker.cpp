#include "SignalBlocker.h"

SignalBlocker::SignalBlocker(QObject* object)
	: mObject(object)
{
	Q_ASSERT(mObject != nullptr);
	Q_ASSERT(!mObject->signalsBlocked());
	mObject->blockSignals(true);
}

SignalBlocker::~SignalBlocker()
{
	mObject->blockSignals(false);
}

