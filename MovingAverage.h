#pragma once
#include <QtCore>

class MovingAverage
{
public:
	MovingAverage(int historySize = 15);
	void setHistorySize(int newSize);

	void update(double newValue);

	double mean() const { return mMean; }
	double peak() const { return mPeak; }
	double trough() const { return mTrough; }

private:
	int mHistorySize;

	QList<double> mValues;
	double mMean;
	double mPeak;
	double mTrough;
};