#pragma once
#include <QtCore>
#include <QtWidgets>
#include "BackgroundImageLoader.h"
#include "MovingAverage.h"
#include "HighPrecisionTimer.h"


class ImageSequencePlayer : public QLabel
{
	Q_OBJECT;

public:
	ImageSequencePlayer(QWidget* parent = nullptr);

	/// if \p nameFilter is empty list then the existing name filters on \p dir
	/// will be left unchanged.
	/// Do not call from a different thread than start/stop
	void setImageSequence(QDir dir, QStringList nameFilters = { "*.png", "*.jpg" });
	void setIsDebugDisplay(bool isDebugDisplay);

	void beginFadeout(int fadeoutDurationMs);

	double framerate() const { return mFramerate; }

public Q_SLOTS:
	void start(double fadeinTimeMs);
	void stop();
	void setFramerate(double framesPerSecond);

protected Q_SLOTS:
	void timerCallback();

private:
	QSize validateFiles(QStringList &files, QDir const& dir);
	/// Get ready for start() to be called
	void prepare();
	void updateDuration();
	void setCurrentlyShownFrame(int frameNumber);

	// user set
	QStringList mFiles;
	double mFramerate;

	// cached
	double mDuration;

	// state
	QTime mStartTime;
	int mCurrentFrameNumber;
	bool mIsPlaying;
	QElapsedTimer mFramerateTimer;
	bool mIsFadeout;
	QTime mFadeoutStartTime;
	double mFadeoutDurationMs;
	double mFadeinDurationMs;

	// objects
	BackgroundImageLoader* mLoader;
	HighPrecisionTimer* mTimer;
	/// is null unless debug display is enabled
	QLabel* mFpsDisplay;
	MovingAverage<int> mFrameDeltaMovingAverage;
    int mFrameDeltaMovingAverageValue;
	QGraphicsOpacityEffect* mOpacityEffect;
};