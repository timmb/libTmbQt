#include "BackgroundImageLoader.h"
#include "Utilities.h"



/// Runnable object to pass to thread pool
class BackgroundImageLoaderRunner : public QRunnable
{
public:
	/// Will call \p receipient's onImageLoaded slot when done
	BackgroundImageLoaderRunner(QString imageFile, int imageIndex, BackgroundImageLoader* receipient)
		: mImageFile(std::move(imageFile))
		, mImageIndex(imageIndex)
		, mReceipient(receipient)
	{
		Q_ASSERT(mReceipient != nullptr);
	}

	virtual void run() override
	{
		QFile file(mImageFile);
		QByteArray data;
		{
			QMutexLocker lock(&sFileIoMutex);
			file.open(QFile::ReadOnly);
			data = file.readAll();
		}
		QImage image = QImage::fromData(data);
		mReceipient->onImageLoaded(mImageIndex, std::move(image));
	}

private:
	static QMutex sFileIoMutex;

	QString mImageFile;
	int mImageIndex;
	BackgroundImageLoader* mReceipient;
};

QMutex BackgroundImageLoaderRunner::sFileIoMutex;



BackgroundImageLoader::BackgroundImageLoader(QObject* parent /*= nullptr*/)
	: QObject(parent)
	, mIsDestructing(false)
{
}

BackgroundImageLoader::~BackgroundImageLoader()
{
	mIsDestructing = true;
	blockUntilAllThreadsComplete();
}

void BackgroundImageLoader::setImageSequence(QVector<QString> imagePaths)
{
	qDebug() << "Waiting for all BackgroundImageLoader threads to complete.";
	mImageSequenceFiles = std::move(imagePaths);
}

void BackgroundImageLoader::bufferImages(QList<int> const& imagesToBuffer)
{
	QMutexLocker lock(&mMutex);
	if (mIsDestructing)
	{
		qDebug() << "Ignoring request to buffer images as destructor has already been called on background image loader.";
	}
	// Purge unwanted images
	for (auto it = mBufferedImages.begin(); it != mBufferedImages.end();)
	{
		if (imagesToBuffer.contains(it.key()))
		{
			++it;
		}
		else
		{
			it = mBufferedImages.erase(it);
		}
	}
	// Buffer new images
	for (int imageIndex : imagesToBuffer)
	{
		Q_ASSERT(0 <= imageIndex && imageIndex < mImageSequenceFiles.size());
		if (!(0 <= imageIndex && imageIndex < mImageSequenceFiles.size()))
		{
			qCritical() << "Out of bounds image index passed to BackgroundImageLoader::bufferImages:" << imageIndex;
			continue;
		}
		if (!(mImagesBeingBuffered.contains(imageIndex) || mBufferedImages.contains(imageIndex)))
		{
			Q_ASSERT(!mImagesBeingBuffered.contains(imageIndex));
			Q_ASSERT(!mBufferedImages.contains(imageIndex));
			/// will delete automatically
			QThreadPool::globalInstance()->start(new BackgroundImageLoaderRunner(mImageSequenceFiles.at(imageIndex), imageIndex, this));
			mImagesBeingBuffered << imageIndex;
		}
	}
}


QImage BackgroundImageLoader::getImage(int imageIndex)
{
	QMutexLocker lock(&mMutex);
	Q_ASSERT(0 <= imageIndex && imageIndex < mImageSequenceFiles.size());
	if (imageIndex < 0 || imageIndex >= mImageSequenceFiles.size())
	{
		qCritical() << "Out of bounds index requested of BackgroundImageLoader.";
		return QImage();
	}

	auto it = mBufferedImages.find(imageIndex);
	// image shouldn't be ready and also being buffered simultaneously
	Q_ASSERT(it == mBufferedImages.end() || !mImagesBeingBuffered.contains(imageIndex));

	// Case 1: image is being buffered but not yet ready
	if (mImagesBeingBuffered.contains(imageIndex))
	{
		// Wait until it's ready then fall through to next case
		while (it == mBufferedImages.end())
		{
			Q_ASSERT(mImagesBeingBuffered.contains(imageIndex) || mBufferedImages.contains(imageIndex));
			mWaitCondition.wait(&mMutex);
			it = mBufferedImages.find(imageIndex);
		}
		// fallthrough...
	}

	// Case 2: image is buffered and waiting
	if (it != mBufferedImages.end())
	{
		return it.value();
	}
	
	// Case 3: image was never buffered. Just load it on this thread. Don't use a thread
	// as in the case of buffer overflow, images returned by threads may be discarded before
	// they are read which would cause a deadlock if we blocked until it arrived.
	else 
	{
		Q_ASSERT(it == mBufferedImages.end() && !mImagesBeingBuffered.contains(imageIndex) && !mBufferedImages.contains(imageIndex));
		return QImage(mImageSequenceFiles.at(imageIndex));
	}
}


void BackgroundImageLoader::onImageLoaded(int index, QImage image)
{
	QMutexLocker lock(&mMutex);
	Q_ASSERT(mImagesBeingBuffered.count(index) == 1);
	Q_ASSERT(!mBufferedImages.contains(index));
	mImagesBeingBuffered.removeOne(index);
	mBufferedImages.insert(index, std::move(image));
	mWaitCondition.wakeAll();
}

void BackgroundImageLoader::blockUntilAllThreadsComplete()
{
	qDebug() << "Waiting for all BackgroundImageLoader threads to complete.";
	QMutexLocker lock(&mMutex);
	while (!mImagesBeingBuffered.isEmpty())
	{
		mWaitCondition.wait(&mMutex);
	}

}

