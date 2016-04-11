#pragma once

#include "QPlainTextEdit"
#include "LogListener.h"
#include "Logger.h"

class LogViewer : public QWidget, public LogListener
{
	Q_OBJECT;

public:
	LogViewer(Logger* logger, QWidget* parent = nullptr);
	virtual ~LogViewer();

	void setLevel(QtMsgType level);
	virtual QtMsgType getLevel() const override;
	virtual void handle(LogMessage const& message) override;

protected Q_SLOTS:
	//void localSettingChanged(QString const& key, QVariant const& value);
	/// process buffer
	void update();

private:
	void refresh();
	/// requires lock
	void p_handle(LogMessage const& message);

	QPlainTextEdit* mConsole;
	Logger* mLogger;

	QtMsgType mLevel;

	mutable QMutex mMutex;
	/// Buffer of html to add to mConsole
	QStringList mBuffer;
};

inline QtMsgType LogViewer::getLevel() const
{
	return mLevel;
}