#pragma once

#include <QApplication>
#include "LogListener.h"
#include "QVector"
#include "LogMessage.h"
#include <QtCore>
#include <memory>



class Logger : public QObject
{
public:
	Logger(QObject* parent=nullptr);
	virtual ~Logger();

	/// \return All past log messages up to max buffer size
	QList<std::shared_ptr<LogMessage const>> getHistory() const;
	/// \return All past error messages up to max buffer size
	QList<std::shared_ptr<LogMessage const>> getErrorHistory() const;

	// set this instance as QT's message handler
	void install();
	void uninstall();

	void addListener(LogListener* logListener);
	void removeListener(LogListener* logListener);

	void handleMessage(std::shared_ptr<LogMessage const> message);

	/// Messages with locations that start with \p location will be ignored completely.
	void ignoreLocation(QString location);
    /// Messages ending with \p suffix will be ignored completely
    void ignoreSuffix(QString suffix);

private:

	QVector<LogListener*> mListeners;
	
	QVector<QString> mLocationsToIgnore;
    QVector<QString> mSuffixesToIgnore;

	/// Remember past messages
	QList<std::shared_ptr<LogMessage const>> mBuffer;
	QList<std::shared_ptr<LogMessage const>> mErrorBuffer;
	mutable QReadWriteLock mBufferMutex;
    mutable QMutex mStdOutMutex;
};

