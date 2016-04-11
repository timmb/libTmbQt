#pragma once
#include "LogListener.h"
#include <QtCore>
#include "Logger.h"

class LogWriter : public QObject, public LogListener
{
	Q_OBJECT;

public:
	LogWriter(Logger* logger, QObject* parent = nullptr);
	virtual ~LogWriter();

	virtual void handle(LogMessage const& message) override;
	virtual QtMsgType getLevel() const { return mLevel; }
	
	void setLevel(QtMsgType level);

protected Q_SLOTS:
	//void onSettingChanged(QString const& key, QVariant const& value);

private:
	void purgeOldLogs();

	QDir mOutputDir;
	QFile* mOutputFile;
	Logger* mLogger;
	QtMsgType mLevel;
	bool mIsHandlingMessage;
};