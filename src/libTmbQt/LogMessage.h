#pragma once
#include <QtCore>

class LogMessage
{
public:
	QDateTime time;
	QtMsgType type;
	QString location;
	QString message;

	LogMessage(QtMsgType type, QMessageLogContext const& context, QString message);
};

QString const& toString(QtMsgType type);