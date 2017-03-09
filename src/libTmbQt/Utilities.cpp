#include "Utilities.h"

QString toString(QDateTime const& date, bool includeTimeZone, QString timeDelimiter)
{
	QString s = date.toString(QString("yyyy-MM-dd HH%1mm%1ss").arg(timeDelimiter));
	if (includeTimeZone)
	{
		s += " "+QTimeZone(date.offsetFromUtc()).displayName(QTimeZone::GenericTime, QTimeZone::OffsetName);
	}
	return s;
}

QString dateString(QString timeDelimiter)
{
	return toString(QDateTime::currentDateTime(), false, timeDelimiter);
}


QString formatDuration(qint64 seconds)
{
	int const MINUTE = 60;
	int const HOUR = 60 * MINUTE;
	int const DAY = 24 * HOUR;
	
	int days = seconds / DAY;
	seconds %= DAY;
	int hours = seconds / HOUR;
	seconds %= HOUR;
	int minutes = seconds / MINUTE;
	seconds %= MINUTE;

	if (days != 0)
	{
		return QString("%1d %2:%3:%4").arg(QString::number(days), QString::number(hours)).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
	}
	else
	{
		return QString("%1:%2:%3").arg(hours).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
	}
}

bool isAsSeriousAs(QtMsgType type, QtMsgType level)
{
	switch (type)
	{
	case QtDebugMsg:
		return level == QtDebugMsg;
	case QtInfoMsg:
		return level == QtDebugMsg || level == QtInfoMsg;
	case QtWarningMsg:
		return level == QtDebugMsg || level == QtInfoMsg || level == QtWarningMsg;
	case QtCriticalMsg:
		return level != QtFatalMsg;
	case QtFatalMsg:
		return true;
	default:
		Q_ASSERT(false);
		return true;
	}
}
