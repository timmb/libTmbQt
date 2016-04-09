#pragma once
#include <QtCore>

#define VERIFY(expression) { bool result = expression; Q_ASSERT(result); }

QString dateString();
QString toString(QDateTime const& date);
QString formatDuration(qint64 numSeconds);

bool isAsSeriousAs(QtMsgType candidateMessageType, QtMsgType requiredSeriousnessLevel);

inline
int msgTypeToInt(QtMsgType type)
{
	switch (type)
	{
	case QtDebugMsg:
		return 0;
	case QtInfoMsg:
		return 1;
	case QtWarningMsg:
		return 2;
	case QtCriticalMsg:
		return 3;
	case QtFatalMsg:
		return 3;
	default:
		Q_ASSERT(false);
		return 3;
		break;
	}
}

inline
QtMsgType intToMsgType(int index)
{
	switch (index)
	{
	case QtDebugMsg:
		return QtDebugMsg;
	case QtInfoMsg:
		return QtInfoMsg;
	case QtWarningMsg:
		return QtWarningMsg;
	case QtCriticalMsg:
		return QtCriticalMsg;
	case QtFatalMsg:
		return QtFatalMsg;
	default:
		Q_ASSERT(false);
		qWarning() << "Unreadable value for level in LogViewer";
		return QtCriticalMsg;
	}
}

inline
float easeInOut(float currentTime, float initialValue, float changeInValue, float duration) {
	return -changeInValue / 2 * (cos(float(M_PI)*currentTime / duration) - 1) + initialValue;
}

inline float easeInOut(float t)
{
	return easeInOut(t, 0, 1, 1);
}