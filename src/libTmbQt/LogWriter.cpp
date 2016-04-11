#include "LogWriter.h"
#include "Utilities.h"

namespace
{
	QString const DATE_TIME_FORMAT = QStringLiteral("yyyy-MM-dd--HH-mm-ss-zzz");
	QString const DATE_TIME_WILDCARD = QStringLiteral("????-??-??--??-??-??-???");
}

LogWriter::LogWriter(Logger* logger, QObject* parent /*= nullptr*/)
	: QObject(parent)
	, mOutputFile(nullptr)
	, mLogger(logger)
	, mIsHandlingMessage(false)
	, mLevel(QtDebugMsg)
{
	mLogger->addListener(this);
	VERIFY(connect(logger, &QObject::destroyed, [this] { mLogger = nullptr; }));

	//mLevel = intToMsgType(gSynchronisedSettings->value(SYNC_LOG_WRITER_LEVEL).toInt());
	QString outputFilename = QDateTime::currentDateTime().toString(DATE_TIME_FORMAT) + ".log";
	QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).mkpath("logs");
	mOutputDir = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath("logs");
	mOutputFile = new QFile(mOutputDir.filePath(outputFilename), this);
	bool openOk = mOutputFile->open(QFile::WriteOnly);
	if (!openOk)
	{
		qCritical() << "Failed to open" << mOutputDir.filePath(outputFilename) << "for writing.";
	}
	else
	{
		QTextStream(mOutputFile) << "Build date : " __DATE__ " " __TIME__ "\n"
			<< "Time" << '\t'
			<< "Level" << '\t'
			<< "Location" << '\t'
			<< "Message" << '\n';
		qInfo() << "Writing log to" << QFileInfo(*mOutputFile).absoluteFilePath();
	}

	for (std::shared_ptr<LogMessage const> const& message : mLogger->getHistory())
	{
		handle(*message);
	}

	purgeOldLogs();
}

LogWriter::~LogWriter()
{
	if (mLogger != nullptr)
	{
		mLogger->removeListener(this);
	}
}

void LogWriter::handle(LogMessage const& message)
{
	// avoid recursion in case writing to file triggers a new log message
	if (!mIsHandlingMessage && shouldHandle(message.type))
	{
		mIsHandlingMessage = true;
		QString datetime = message.time.toString("yyyy-MM-dd HH:mm:ss.zzz");
		QTextStream(mOutputFile) << datetime << '\t'
			<< toString(message.type) << '\t'
			<< message.location << '\t'
			<< message.message << '\n';
		mIsHandlingMessage = false;
	}
}

void LogWriter::setLevel(QtMsgType level)
{
	mLevel = level;
}

//void LogWriter::onSettingChanged(QString const& key, QVariant const& value)
//{
//	if (key == SYNC_LOG_WRITER_LEVEL)
//	{
//		Q_ASSERT(value.type() == QVariant::Int);
//		mLevel = intToMsgType(value.toInt());
//	}
//	else if (key == SYNC_LOG_PURGE_AGE_DAYS)
//	{
//		Q_ASSERT(value.type() == QVariant::Int);
//		qInfo() << "Logs older than" << value << "days will be purged next time the application is launched.";
//	}
//}

void LogWriter::purgeOldLogs()
{
	bool isPurgeEnabled = false;// gSynchronisedSettings->value(SYNC_IS_LOG_PURGING_ENABLED).toBool();

	if (isPurgeEnabled)
	{
		//Q_ASSERT(gSynchronisedSettings->value(SYNC_LOG_PURGE_AGE_DAYS).type() == QVariant::Int);
		int maxDaysOld = 100;// gSynchronisedSettings->value(SYNC_LOG_PURGE_AGE_DAYS).toInt();
		if (maxDaysOld < 0)
		{
			//qWarning() << "Value for" << SYNC_LOG_PURGE_AGE_DAYS << "was negative. Resetting to default.";
			//gSynchronisedSettings->setValue(SYNC_LOG_PURGE_AGE_DAYS, gSynchronisedSettings->defaultValue(SYNC_LOG_PURGE_AGE_DAYS));
			//maxDaysOld = gSynchronisedSettings->value(SYNC_LOG_PURGE_AGE_DAYS).toInt();
			Q_ASSERT(maxDaysOld >= 0);
		}
		QDateTime const minLogDate = QDateTime::currentDateTime().addDays(-maxDaysOld);
		QStringList const logFiles = mOutputDir.entryList(QStringList(DATE_TIME_WILDCARD + ".log"), QDir::Files | QDir::Writable, QDir::Time);
		{
			for (QString const& logFilePath : logFiles)
			{
				QDateTime logTime = QDateTime::fromString(logFilePath.left(DATE_TIME_FORMAT.length()), DATE_TIME_FORMAT);
				if (logTime < minLogDate)
				{
					if (QFileInfo(*mOutputFile) == QFileInfo(logFilePath))
					{
						qDebug() << "Not purging log file" << logFilePath << "as that is what we're currently logging to.";
					}
					else
					{
						qInfo() << "Purging log file" << logFilePath << "as date is less than minimum log date of" << minLogDate.toString(DATE_TIME_FORMAT)
							<< qPrintable(QString("(%1 days ago)").arg(maxDaysOld));
						QFile logFile(mOutputDir.filePath(logFilePath));
						if (!logFile.remove())
						{
							qCritical() << "Failed to delete log file:" << logFilePath << "-" << logFile.errorString();
						}
					}
				}
			}
		}
	}
	else
	{
		qWarning() << "Purging of old log files is disabled. It is advised to enable this "
			"to prevent the hard drive from slowly filling up and causing problems in the future.";
	}
}

