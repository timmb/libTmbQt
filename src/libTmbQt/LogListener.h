#pragma once

#include "qlogging.h"
#include "LogMessage.h"

class LogListener
{
public:
	virtual QtMsgType getLevel() const = 0;
	
	bool shouldHandle(QtMsgType type) const;
	virtual void handle(LogMessage const& logMessage) = 0;
};