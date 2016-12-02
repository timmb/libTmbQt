#include "FramerateStats.h"

namespace
{
bool const _ = [] { qRegisterMetaType<FramerateStats>(); return true; }();
}

FramerateStats::FramerateStats(int lastDuration_, int meanDuration_, int peakDuration_, int lastDelta_, int meanDelta_, int peakDelta_)
: lastDuration(lastDuration_)
, meanDuration(meanDuration_)
, peakDuration(peakDuration_)
, lastDelta(lastDelta_)
, meanDelta(meanDelta_)
, peakDelta(peakDelta_)
{}

FramerateStats()
: lastDuration(-42)
, meanDuration(-42)
, peakDuration(-42)
, lastDelta(-42)
, meanDelta(-42)
, peakDelta(-42)
{}
