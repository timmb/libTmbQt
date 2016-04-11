#include "LogMessage.h"
#include <xutility>

LogMessage::LogMessage(QtMsgType type_, QMessageLogContext const& context_, QString message_)
	: time(QDateTime::currentDateTime())
	, type(type_)
	, location((context_.file && context_.file[0] == '\0') ? QString() : QString("%1:%2, %3").arg(QFileInfo(context_.file).fileName(), QString::number(context_.line), QString::fromUtf8(context_.function).replace("__cdecl ","")))
	, message(std::move(message_))
{

}


QString const& toString(QtMsgType type)
{
	static QString Debug("Debug");
	static QString Info("Info");
	static QString Warning("Warning");
	static QString Critical("Critical");
	static QString Fatal("Fatal");

	switch (type)
	{
	case QtDebugMsg:
		return Debug;
	case QtInfoMsg:
		return Info;
	case QtWarningMsg:
		return Warning;
	case QtCriticalMsg:
		return Critical;
	default:
	case QtFatalMsg:
		return Fatal;
	}
}