#include "ImageSequencePlayer.h"
#include "Utilities.h"
#include <unordered_set>
#include <iostream>

namespace
{
	int const LOOK_AHEAD = 3;
}


ImageSequencePlayer::ImageSequencePlayer(QWidget* parent /*= nullptr*/)
	: QLabel(parent)
	, mFramerate(30)
	, mDuration(0)
	, mCurrentFrameNumber(-1)
	, mIsPlaying(false)
	, mIsFadeout(false)
	, mLoader(new BackgroundImageLoader(this))
	, mTimer(new HighPrecisionTimer(this))
	, mFpsDisplay(nullptr)
	, mFrameDeltaMovingAverage(30)
	, mOpacityEffect(new QGraphicsOpacityEffect)
{
	//mTimer->setTimerType(Qt::PreciseTimer);
	//mTimer->setSingleShot(true);
	VERIFY(connect(mTimer, SIGNAL(timeout()), this, SLOT(timerCallback())));
	setGraphicsEffect(mOpacityEffect);
}

void ImageSequencePlayer::setImageSequence(QDir dir, QStringList nameFilters)
{
	// TODO: Check files types that can be read
	if (!nameFilters.isEmpty())
	{
		dir.setNameFilters(nameFilters);
	}
	qDebug() << "Loading files from" << dir.path() << "matching" << dir.nameFilters();
	auto files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase);
	// convert to absolute path
	{
		for (QString& file : files)
		{
			file = dir.absoluteFilePath(file);
		}
	}

	QSize detectedSize = validateFiles(files, dir);

	resize(detectedSize);
	if (files.isEmpty())
	{
		qWarning() << "No files found in" + dir.absolutePath();
		stop();
	}
	mFiles = std::move(files);
	mLoader->setImageSequence(mFiles.toVector());
	prepare();
}

QSize ImageSequencePlayer::validateFiles(QStringList &files, QDir const& dir)
{
	QSize detectedSize;
	bool isInconsistentSizeDetected(false);
	QProgressDialog* progress = new QProgressDialog("Validating image sequence in " + dir.path(), "Skip", 0, qMax(1, files.size()), this);
	progress->setWindowModality(Qt::WindowModal);
	// test for non/bad images
	int i = 0;
	for (auto it = files.begin(); it != files.end();)
	{
		QString& file = *it;
		QImage image(file);
		QSize size = image.size();
		if (image.isNull())
		{
			qWarning() << "Unable to open or read as image:" << file;
			it = files.erase(it);
		}
		else if (size.isEmpty() || size.isNull())
		{
			qWarning() << "Image with size (0,0) found and ignored:" << file;
			it = files.erase(it);
		}
		else
		{
			if (detectedSize.isNull() || detectedSize.isEmpty())
			{
				detectedSize = size;
			}
			else if (detectedSize != size)
			{
				isInconsistentSizeDetected = true;
			}
			++it;
		}
		//qDebug() << "Found image sequence file:" << file;
		progress->setValue(i++);
		if (progress->wasCanceled())
		{
			qInfo() << "Validation of image sequence in" << dir.absolutePath() << "was skipped.";
			break;
		}
	}
	progress->setValue(progress->maximum());
	if (isInconsistentSizeDetected)
	{
		qWarning() << "Image sequence files do not all have same size.";
	}
	return detectedSize;
}

void ImageSequencePlayer::prepare()
{
	updateDuration();
	int n = qMin(LOOK_AHEAD, mFiles.size());
	QList<int> imagesToBuffer;
	for (int i = 0; i < n; i++)
	{
		imagesToBuffer << i;
	}
	mLoader->bufferImages(imagesToBuffer);
}

void ImageSequencePlayer::start(double fadeinTimeMs)
{
	if (!mFiles.isEmpty())
	{
		mFadeinDurationMs = fadeinTimeMs;
		mIsFadeout = false;
		mOpacityEffect->setOpacity(1.0);
		mIsPlaying = true;
		mStartTime.start();
		mFramerateTimer.restart();
		timerCallback();
	}
}

void ImageSequencePlayer::stop()
{
	mIsPlaying = false;
	//mTimer->stop();
	prepare();
}

void ImageSequencePlayer::setFramerate(double framesPerSecond)
{
	Q_ASSERT(framesPerSecond > 0);
	mFramerate = qMax(mFramerate, 1.0e-10);
	updateDuration();

	//mTimer->setInterval(int(1 / mFramerate));
}

void ImageSequencePlayer::timerCallback()
{
	if (mIsPlaying)
	{
		Q_ASSERT(!mFiles.isEmpty());
		if (mFiles.isEmpty())
		{
			qCritical() << "Frame update called with empty files list.";
			return;
		}
		int msSinceStart = mStartTime.elapsed();
		double sSinceStart = msSinceStart / 1000.0;
		int framesSinceStart = int(sSinceStart * mFramerate);
		int currentFrame = framesSinceStart % mFiles.size();
		if (currentFrame != mCurrentFrameNumber)
		{
			setCurrentlyShownFrame(currentFrame);
		}
		int nextFrame = framesSinceStart + 1;
		double sSinceStartForNextFrame = nextFrame / mFramerate;
		double sUntilNextFrame = sSinceStartForNextFrame - sSinceStart;
		int msUntilNextFrame = int(sUntilNextFrame * 1000.0);
		mTimer->start(msUntilNextFrame);
		//qDebug() << "Setting callback for next frame to arrive in (ms):" << msUntilNextFrame;
	}
}


void ImageSequencePlayer::updateDuration()
{
	mDuration = mFiles.size() / mFramerate;
}

void ImageSequencePlayer::setCurrentlyShownFrame(int frameNumber)
{
	Q_ASSERT(0 <= frameNumber && frameNumber < mFiles.size());
	mCurrentFrameNumber = frameNumber;

	QImage image = mLoader->getImage(frameNumber);
	if (image.isNull())
	{
		qCritical() << "Unable to load image file:" << mFiles.at(frameNumber);
	}
	else
	{
		double opacity = mFadeinDurationMs==0? 1.0 : std::min(1.0, mStartTime.elapsed() / mFadeinDurationMs);
		if (mIsFadeout)
		{
			double t = mFadeoutDurationMs ==0? 1.0 : std::min(1.0, mFadeoutStartTime.elapsed() / mFadeoutDurationMs);
			Q_ASSERT(t >= 0.0);
			Q_ASSERT(t <= 1.0);
			opacity = std::min(opacity, 1.0 - t);
		}
		if (opacity<1.0)
		{
			mOpacityEffect->setOpacity(easeInOut(opacity));
			mOpacityEffect->setEnabled(true);
		}
		else
		{
			mOpacityEffect->setEnabled(false);
		}
		setPixmap(QPixmap::fromImage(std::move(image), Qt::NoFormatConversion));
		repaint();
		if (mFpsDisplay != nullptr)
		{
			mFrameDeltaMovingAverage.update(mFramerateTimer.elapsed());
			mFpsDisplay->setText(QString("%4 fps (delta mean: %1ms peak: %2ms, trough: %3ms)")
				.arg(int(mFrameDeltaMovingAverage.mean()), 3)
				.arg(int(mFrameDeltaMovingAverage.peak()), 3)
				.arg(int(mFrameDeltaMovingAverage.trough()), 3)
				.arg(1000.0 / mFrameDeltaMovingAverage.mean(), 3, 'f', 0));
			mFramerateTimer.restart();
		}
	}
	QList<int> imagesToBuffer;
	imagesToBuffer.reserve(LOOK_AHEAD);
	for (int i = 1; i <= LOOK_AHEAD; i++)
	{
		int n = (frameNumber + i) % mFiles.size();
		imagesToBuffer << n;
		if (n == frameNumber)
		{
			// don't duplicate files
			break;
		}
	}
	mLoader->bufferImages(imagesToBuffer);
}


void ImageSequencePlayer::setIsDebugDisplay(bool isDebugDisplay)
{
	if (isDebugDisplay && mFpsDisplay == nullptr)
	{
		mFpsDisplay = new QLabel(this);
		mFpsDisplay->resize(size());
		mFpsDisplay->setAlignment(Qt::AlignRight | Qt::AlignBottom);
		mFpsDisplay->setContentsMargins(5, 5, 5, 5);
		mFpsDisplay->show();
	}
	else if (!isDebugDisplay && mFpsDisplay != nullptr)
	{
		delete mFpsDisplay;
		mFpsDisplay = nullptr;
	}
}

void ImageSequencePlayer::beginFadeout(int fadeoutDurationMs)
{
	mIsFadeout = true;
	mFadeoutDurationMs = fadeoutDurationMs;
	mFadeoutStartTime.start();
}

