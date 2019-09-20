#include "HighPrecisionTimer.h"
#include "Utilities.h"


HighPrecisionTimerInner::HighPrecisionTimerInner()
	: mIsCancelled(false)
{

}

void HighPrecisionTimerInner::startTimer()
{
	mIsCancelled = false;
	mTimer.start();
}

void HighPrecisionTimerInner::cancel()
{
	mIsCancelled = true;
}

void HighPrecisionTimerInner::startLoop(double milliseconds)
{
	//Q_ASSERT(mTimer.isValid());
    const qint64 nanoseconds = (qint64) qFloor(milliseconds * 1000000.0);
	while (mTimer.nsecsElapsed() < nanoseconds && !mIsCancelled)
	{
		QThread::usleep(50);
	}
	//mTimer.invalidate();
	if (mIsCancelled)
	{
		return;
	}
	Q_EMIT timeout();

}




HighPrecisionTimer::HighPrecisionTimer(QObject* parent /*= nullptr*/)
	: QObject(parent)
	, mThread(new QThread(this))
	, mInner(new HighPrecisionTimerInner)
	, mIsRunning(false)
{
	VERIFY(connect(this, SIGNAL(startInner(double)), mInner, SLOT(startLoop(double))));
	VERIFY(connect(mInner, SIGNAL(timeout()), this, SLOT(timeoutInner())));

	mThread->start(QThread::TimeCriticalPriority);
	mInner->moveToThread(mThread);

}

HighPrecisionTimer::~HighPrecisionTimer()
{
    mThread->quit();
	cancel();
    VERIFY(mThread->wait(5000));
}


void HighPrecisionTimer::cancel()
{
    mIsRunning = false;
	mInner->cancel();
}

void HighPrecisionTimer::start(double milliseconds)
{
	if (mIsRunning)
	{
		cancel();
	}
	mIsRunning = true;
	mInner->startTimer();
	Q_EMIT startInner(milliseconds);
}

void HighPrecisionTimer::timeoutInner()
{
    if (mIsRunning)
    {
        mIsRunning = false;
        Q_EMIT timeout();
    }
}



