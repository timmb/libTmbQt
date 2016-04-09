#include "MovingAverage.h"

MovingAverage::MovingAverage(int historySize /*= 15*/)
	: mHistorySize(historySize)
	, mMean(0)
	, mPeak(0)
	, mTrough(0)
{
	mHistorySize = qMax(mHistorySize, 1);
}

void MovingAverage::setHistorySize(int newSize)
{
	mHistorySize = qMax(1, newSize);
}

void MovingAverage::update(double newValue)
{
	Q_ASSERT(mHistorySize > 0);
	mValues << newValue;
	while (mValues.size() > mHistorySize)
	{
		mValues.pop_front();
	}
	Q_ASSERT(!mValues.isEmpty());
	mPeak = -std::numeric_limits<double>::infinity();
	mTrough = std::numeric_limits<double>::infinity();
	mMean = 0;
	for (double d : mValues)
	{
		mPeak = qMax(mPeak, d);
		mTrough = qMin(mTrough, d);
		mMean += d;
	}
	mMean /= mValues.size();
	Q_ASSERT(mPeak == newValue || mPeak != -std::numeric_limits<double>::infinity());
	Q_ASSERT(mTrough == newValue || mTrough != std::numeric_limits<double>::infinity());
}


