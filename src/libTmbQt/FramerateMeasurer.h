#pragma once
#include <QtCore>
#include "MovingAverage.h"
#include "MovingPeak.h"
#include "FramerateStats.h"

class FramerateMeasurer : public QObject
{
    Q_OBJECT;
public:
    FramerateMeasurer(int windowLength, QObject* parent=nullptr);
    
    /// Call at the start of each frame
    void beginFrame();
    /// Call at the end of each frame
    void endFrame();
    
    int lastFrameDurationMs() const { return mLastDuration; }
    int meanFrameDurationMs() const { return mMeanDuration; }
    int peakFrameDurationMs() const { return mPeakDuration; }
    
    int lastFrameDeltaMs() const { return mLastDelta; }
    int meanFrameDeltaMs() const { return mMeanDelta; }
    int peakFrameDeltaMs() const { return mPeakDelta; }
    
    /// Set number of samples used in peak and mean calculations
    void setWindowLength(int numSamples);
    
Q_SIGNALS:
    void framerateChanged(FramerateStats const&);
//    void framerateChanged(int lastFrameDuration, int meanFrameDuration, int peakFrameDuration, int lastFrameDelta, int meanFrameDelta, int peakFrameDelta);

    
private:
    QElapsedTimer mTimer;
    MovingAverage<int> mMeanFrameDuration;
    MovingPeak<int> mPeakFrameDuration;
    MovingAverage<int> mMeanFrameDelta;
    MovingPeak<int> mPeakFrameDelta;
    int mLastDuration;
    int mMeanDuration;
    int mPeakDuration;
    int mLastDelta;
    int mMeanDelta;
    int mPeakDelta;
};
