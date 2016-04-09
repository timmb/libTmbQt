#pragma once
#include <QtCore>
#include <QImage>


///// Signalling object to run in a different thread
//class BackgroundImageLoaderWorker : public QObject
//{
//	Q_OBJECT;
//
//public:
//	BackgroundImageLoaderWorker(QString filename);
//
//	void run();
//
//private:
//	QString mFilename;
//};


class BackgroundImageLoaderRunner;


class BackgroundImageLoader : public QObject
{
	Q_OBJECT;
public:
	/// Move this object to a thread to make it a background image loader
	BackgroundImageLoader(QObject* parent = nullptr);
	virtual ~BackgroundImageLoader();

	/// Set image sequence. Future calls refer to indices in this list.
	/// This will block until all buffering is finished
	void setImageSequence(QVector<QString> imagePaths);

	/// Tell this class which images should set to be buffered. This will not retrigger
	/// a rebuffering if a given image is already being processed from a previous request.
	/// Previously buffered images not in this list will be purged.
	void bufferImages(QList<int> const& imageIndices);

	/// \return an image from file. If this was previously buffered then buffered
	/// version is returned and not retained. If unbuffered then the call blocks
	/// while it loads. If it's queued to be buffered but not yet buffered then
	/// this call blocks until it is processed.
	/// If the image cannot be loaded then returns a null QImage.
	QImage getImage(int imageIndex);

private:
	/// Called by the worker thread
	void onImageLoaded(int index, QImage image);

	/// \pre mMutex is not locked by the current thread
	void blockUntilAllThreadsComplete();

	QVector<QString> mImageSequenceFiles;
	/// Keep track of threads that are running, and how many requests there are for that
	/// image
	QList<int> mImagesBeingBuffered;
	/// Images returned by threadpool, and how many requests remain for
	/// that image
	QMap<int, QImage> mBufferedImages;
	//QVector<QThread> mThreadPool;
	//int mNextThread;
	mutable QMutex mMutex;
	QWaitCondition mWaitCondition;
	bool mIsDestructing;

	friend class BackgroundImageLoaderRunner;
};