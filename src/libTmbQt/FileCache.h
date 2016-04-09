#pragma once
#include <QtCore>
#include <QtNetwork>

class FileCache : public QObject
{
	Q_OBJECT;

public:
	FileCache(QObject* parent = nullptr);
	/// Purges the cache
	virtual ~FileCache();

	//void setCacheDirectory(QString cacheDirectory);
	//QString cacheDirectory() const { return mCacheDirectory; }

	/// default is 500 000 000
	/// If the cache reaches this a warning is printed and no
	/// more files are cached
	void setMaximumCacheSize(qint64 maximumBytes);

	bool isCached(QString const& url) const { return mCache.contains(url); }

	/// If url is cached it will return a file:/// link for the url.
	/// Otherwise it will return the url itself and start caching the
	/// url for the next time hopefully.
	QString getCachedUrl(QString const& url);


protected Q_SLOTS:
	void onNetworkReply(QNetworkReply* reply);

private:
	/// Mutex must be locked when calling this.
	/// will purge if too big
	void checkCacheSize();

	//QString mCacheDirectory;
	/// remote url -> file:// url of locally cached-version
	QMap<QUrl, QTemporaryFile*> mCache;
	QList<QUrl> mUrlsBeingCached;
	qint64 mMaximumCacheSize;

	QNetworkAccessManager* mManager;

	mutable QMutex mMutex;
};