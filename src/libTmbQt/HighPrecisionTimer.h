#pragma once
#include <QtCore>
#include <atomic>

class HighPrecisionTimerInner : public QObject
{
	Q_OBJECT;
private:
	HighPrecisionTimerInner();

	/// Sets start time
	void startTimer();
	void cancel();

protected Q_SLOTS:
	/// Starts thread loop
	void startLoop(double milliseconds);

Q_SIGNALS:
	void timeout();

private:
	QElapsedTimer mTimer;
	bool mIsCancelled;

	friend class HighPrecisionTimer;
};


class HighPrecisionTimer : public QObject
{
	Q_OBJECT;
public:
	HighPrecisionTimer(QObject* parent = nullptr);
	virtual ~HighPrecisionTimer();
	
	/// Calling start() while isRunning is true cause timer to wait
	/// for previous run to finish
	bool isRunning() const { return mIsRunning; }

	/// Prevent any pending timeout
	void cancel();
    

public Q_SLOTS:
	/// Start one-shot timer
	void start(double milliseconds);

protected Q_SLOTS:
	void timeoutInner();

Q_SIGNALS:
	void timeout();

	void startInner(double milliseconds);

private:
	QThread* mThread;
	HighPrecisionTimerInner* mInner;

	std::atomic<bool> mIsRunning;
};
