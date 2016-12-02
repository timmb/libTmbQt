#include "FramerateMeasurer.h"


    FramerateMeasurer::FramerateMeasurer(int windowLength, QObject* parent)
    : QObject(parent)
    , mLastDuration(windowLength)
    , mMeanDuration(windowLength)
    , mPeakDuration(windowLength)
    , mLastDelta(windowLength)
    , mMeanDelta(windowLength)
    , mPeakDelta(windowLength)
    {
        mMeanFrameDelta.mNumSamples = windowLength;
        mPeakFrameDelta.numSamples = windowLength;
        mMeanFrameDuration.mNumSamples = windowLength;
        mPeakFrameDuration.numSamples = windowLength;
        mTimer.start();
    }
    
    void FramerateMeasurer::beginFrame()
    {
        mLastDelta = (int) mTimer.restart();
        mMeanDelta = mMeanFrameDelta.update(mLastDelta);
        mPeakDelta = mPeakFrameDelta.update(mLastDelta);
    }
    
    
    void FramerateMeasurer::endFrame()
    {
        mLastDuration = (int) mTimer.elapsed();
        mMeanDuration = mMeanFrameDuration.update(mLastDuration);
        mPeakDuration = mPeakFrameDuration.update(mLastDuration);
        Q_EMIT framerateChanged({mLastDuration, mMeanDuration, mPeakDuration, mLastDelta, mMeanDelta, mPeakDelta});
    }
    
    void FramerateMeasurer::setWindowLength(int windowLength)
    {
        mMeanFrameDelta.mNumSamples = windowLength;
        mPeakFrameDelta.numSamples = windowLength;
        mMeanFrameDuration.mNumSamples = windowLength;
        mPeakFrameDuration.numSamples = windowLength;
        
    }
