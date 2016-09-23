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
	virtual QtMsgType getLevel() const override { return mLevel; }

    void setLevel(QtMsgType level);
    void purgeOldLogs(int maxAgeInDays = 100);
    

protected Q_SLOTS:
    
	//void onSettingChanged(QString const& key, QVariant const& value);

private:
	QDir mOutputDir;
	QFile* mOutputFile;
	Logger* mLogger;
	QtMsgType mLevel;
	bool mIsHandlingMessage;
};
