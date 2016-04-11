#include "LogListener.h"
#include "libTmbQt/Utilities.h"



bool LogListener::shouldHandle(QtMsgType type) const
{
	return isAsSeriousAs(type, getLevel());
}


