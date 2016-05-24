#include "FileCache.h"
#include "Utilities.h"

FileCache::FileCache(QObject* parent /*= nullptr*/)
	: QObject(parent)
	, mMaximumCacheSize(500000000)
	, mManager(new QNetworkAccessManager(this))
	//, mCacheDirectory(QDesktopServices::storageLocation(QStandardPaths::CacheLocation))
{
	VERIFY(connect(mManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onNetworkReply(QNetworkReply*))));
}

FileCache::~FileCache()
{
	// temporary files automatically purge
}

//void FileCache::setCacheDirectory(QString cacheDirectory)
//{
//
//}

void FileCache::setMaximumCacheSize(qint64 maximumBytes)
{
	mMaximumCacheSize = maximumBytes;
}




void FileCache::checkCacheSize()
{
	// check mutex is locked by this thread.
	Q_ASSERT(mMutex.tryLock(0) == false);

	qint64 totalSize(0);
	for (QTemporaryFile* file : mCache.values())
	{
		totalSize += file->size();
	}
	if (totalSize > mMaximumCacheSize)
	{
		qWarning() << QString("Purging cache as size (%1) exceeded maximum (%2)").arg(totalSize).arg(mMaximumCacheSize);
		mCache.clear();
	}
}

QString FileCache::getCachedUrl(QString const& urlString)
{
	Q_ASSERT(!urlString.isEmpty());
	if (urlString.isEmpty())
	{
		return urlString;
	}

	QUrl url(urlString);

	QTemporaryFile const* cacheValue(nullptr);
	bool needToStartCache(false);

	// need to make sure mutex isn't locked when we make the network request in case it signals within the call
	// as our slot will then try to lock the mutex again
	{
		QMutexLocker lock(&mMutex);
		if (mCache.contains(url))
		{
			cacheValue = mCache.value(url);
		}
		if (cacheValue == nullptr && !mUrlsBeingCached.contains(url))
		{
			mUrlsBeingCached << url;
			needToStartCache = true;
		}
	}

	if (cacheValue != nullptr)
	{
		Q_ASSERT(!needToStartCache);
		QString file = QFileInfo(*cacheValue).absoluteFilePath();
		return QUrl::fromLocalFile(file).toString();
	}
	else 
	{
		if (needToStartCache)
		{
			qDebug() << "No cache found for" << url << "- sending network request to cache it.";
			QNetworkRequest request;
			//request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork);
			request.setUrl(url);
			mManager->get(request);
		}
		return urlString;
	}
}

void FileCache::onNetworkReply(QNetworkReply* reply)
{
	QMutexLocker lock(&mMutex);
	if (mCache.contains(reply->url()))
	{
		delete mCache[reply->url()];
		mCache[reply->url()] = nullptr;
	}
	QTemporaryFile* file = mCache[reply->url()] = new QTemporaryFile(this);

	if (!file->open())
	{
		qCritical() << "Failed to open temporary file" << file->fileName();
	}
	else
	{
		if (reply->bytesAvailable() > mMaximumCacheSize)
		{
			qWarning() << "Unable to cache" << reply->url() << "as it is" << reply->bytesAvailable() << "bytes but maximum cache size is" << mMaximumCacheSize << "bytes";
		}
		else
		{
			//while (!reply->atEnd())
			{
				QByteArray data = reply->readAll();
				if (!data.isEmpty())
				{
					file->write(data);
				}
			}
			if (reply->error() == QNetworkReply::NoError && file->error() == QFile::NoError)
			{
				qDebug() << "Cached url" << reply->url() << "to temporary file" << file->fileName();
			}
			if (reply->error() != QNetworkReply::NoError)
			{
				qWarning() << "Error reading network data when attempting to cache url" << reply->url() << ":" << reply->errorString();
				mCache.remove(reply->url());
			}
			if (file->error() != QFileDevice::NoError)
			{
				qCritical() << "Error when attempting to write to temporary cache file" << file->fileName() << ":" << file->errorString();
				mCache.remove(reply->url());
			}
		}
	}
	reply->deleteLater();
	checkCacheSize();

	Q_ASSERT(mUrlsBeingCached.contains(reply->url()));
	mUrlsBeingCached.removeAll(reply->url());
}