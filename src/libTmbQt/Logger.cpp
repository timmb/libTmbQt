#include "Logger.h"
#include <functional>
#include <xutility>
#include <iostream>
#include "libTmbQt/Utilities.h"

namespace
{
	static Logger* gQtMessageHandler = nullptr;
	void handleMessage(QtMsgType type, QMessageLogContext const& context, QString const& msg)
	{
		Q_ASSERT(gQtMessageHandler != nullptr);
		gQtMessageHandler->handleMessage(std::make_shared<LogMessage const>(type, std::move(context), std::move(msg)));
	}

	const int MAX_BUFFER_LENGTH = 20000;
}



Logger::Logger(QObject* parent)
: QObject(parent)
{

}

Logger::~Logger()
{
	uninstall();
}

QList<std::shared_ptr<LogMessage const>> Logger::getHistory() const
{
	QReadLocker lock(&mBufferMutex);
	return mBuffer;
}

QList<std::shared_ptr<LogMessage const>> Logger::getErrorHistory() const
{
	QReadLocker lock(&mBufferMutex);
	return mErrorBuffer;
}

void Logger::install()
{
	if (gQtMessageHandler != nullptr)
	{
		gQtMessageHandler->uninstall();
	}
	gQtMessageHandler = this;
	qInstallMessageHandler(&::handleMessage);
}

void Logger::uninstall()
{
	if (gQtMessageHandler == this)
	{
		qInstallMessageHandler(0);
		gQtMessageHandler = nullptr;
	}
}

void Logger::addListener(LogListener* logListener)
{
	Q_ASSERT(logListener != nullptr);
	mListeners.push_back(logListener);
}

void Logger::removeListener(LogListener* logListener)
{
	mListeners.removeAll(logListener);
}

void Logger::handleMessage(std::shared_ptr<LogMessage const> message)
{
	for (QString const& locationToIgnore : mLocationsToIgnore)
	{
		if (message->location.startsWith(locationToIgnore))
		{
			return;
		}
	}

	for (LogListener* listener : mListeners)
	{
		if (listener->shouldHandle(message->type))
		{
			listener->handle(*message);
		}
	}

	if (mListeners.empty())
	{
		std::cerr << toString(message->time).toLocal8Bit().constData() 
			<< " " << toString(message->type).toLocal8Bit().constData() 
			<< ": " << message->message.toLocal8Bit().constData() 
			<< " (" << message->location.toLocal8Bit().constData() << ")"
			<< std::endl;
	}


	{
		QWriteLocker lock(&mBufferMutex);
		if (isAsSeriousAs(message->type, QtCriticalMsg))
		{
			mErrorBuffer << message;
			while (mErrorBuffer.size() > MAX_BUFFER_LENGTH)
			{
				mErrorBuffer.pop_front();
			}
		}
		mBuffer << std::move(message);
		while (mBuffer.size() > MAX_BUFFER_LENGTH)
		{
			mBuffer.pop_front();
		}
		Q_ASSERT(mBuffer.size() <= MAX_BUFFER_LENGTH);
	}


}

void Logger::ignoreLocation(QString location)
{
	mLocationsToIgnore << std::move(location);
}

