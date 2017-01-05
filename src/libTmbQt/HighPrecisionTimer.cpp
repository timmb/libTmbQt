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

void HighPrecisionTimerInner::startLoop(int milliseconds)
{
	//Q_ASSERT(mTimer.isValid());
	while (mTimer.elapsed() < milliseconds && !mIsCancelled)
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
	VERIFY(connect(this, SIGNAL(startInner(int)), mInner, SLOT(startLoop(int))));
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

void HighPrecisionTimer::start(int milliseconds)
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



