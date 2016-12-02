#pragma once

#include <QtCore>

class FramerateStats
{
public:
    int lastDuration;
    int meanDuration;
    int peakDuration;
    int lastDelta;
    int meanDelta;
    int peakDelta;
    
    FramerateStats(int lastDuration_, int meanDuration_, int peakDuration_, int lastDelta_, int meanDelta_, int peakDelta_);
    FramerateStats();
};
Q_DECLARE_METATYPE(FramerateStats);
