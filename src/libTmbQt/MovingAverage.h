#pragma once

#include <boost/circular_buffer.hpp>
//#include "CinderOpenCV.h"

template <typename T>
class MovingAverage
{
public:
	MovingAverage();
	T update(T const& newValue);
	void clear();
	/// may be nullptr
	T* newest();
	/// may be nullptr
	T* oldest();
	bool isEmpty() const;
	std::vector<T> history() const;
	size_t size() const;

	int mNumSamples;

private:
	/// Sum of data in mBuffer
	T mCurrentSum;
	boost::circular_buffer<T> mBuffer;

	T mCurrentRecalculationSum;
	// how many vaues stored in current recalculation sum
	int mCurrentRecalculationSumCount;
};

template <typename T>
size_t MovingAverage<T>::size() const
{
	return mBuffer.size();
}

template <typename T>
std::vector<T> MovingAverage<T>::history() const
{
	return std::vector<T>(std::begin(mBuffer), std::end(mBuffer));
}

template <typename T>
T clone(T const& rhs)
{
	return rhs;
}

//template <>
//inline
//cv::Mat clone(cv::Mat const& rhs)
//{
//	return rhs.clone();
//}

template <typename T>
MovingAverage<T>::MovingAverage()
: mNumSamples(40)
, mCurrentSum(T())
, mBuffer(40)
, mCurrentRecalculationSum(T())
, mCurrentRecalculationSumCount(0)
{}

template <typename T>
bool MovingAverage<T>::isEmpty() const
{
	return mBuffer.empty();
}

template <typename T>
T* MovingAverage<T>::newest()
{
	return isEmpty() ? nullptr : &mBuffer.back();
}

template <typename T>
T* MovingAverage<T>::oldest()
{
	return isEmpty() ? nullptr : &mBuffer.front();
}

template <typename T>
void MovingAverage<T>::clear()
{
	mBuffer.clear();
	mCurrentRecalculationSum = T();
	mCurrentRecalculationSumCount = 0;
}

template <typename T>
T MovingAverage<T>::update(T const& d)
{
	if (mNumSamples != mBuffer.capacity())
	{
		mBuffer.set_capacity(mNumSamples);
	}

	if (mBuffer.empty())
	{
		mCurrentSum = clone(d);
	}
	else
	{
		mCurrentSum += d;
		if (mBuffer.full())
		{
			mCurrentSum = mCurrentSum - mBuffer.front();
			mBuffer.pop_front();
		}
	}
	mBuffer.push_back(d);
	// Also incrementally recalculate to get rid of rounding
	// errors.
	if (mCurrentRecalculationSumCount == 0)
	{
		mCurrentRecalculationSum = clone(d);
	}
	else
	{
		mCurrentRecalculationSum += d;
	}
	mCurrentRecalculationSumCount++;
	if (mCurrentRecalculationSumCount == mNumSamples && mBuffer.size() == mNumSamples)
	{
		mCurrentSum = mCurrentRecalculationSum;
	}
	if (mCurrentRecalculationSumCount >= mNumSamples)
	{
		mCurrentRecalculationSum = T();
		mCurrentRecalculationSumCount = 0;
	}
	return mCurrentSum / mBuffer.size();
}