#include "CachedData.h"
#include "Utilities.h"

CachedData::CachedData(QString uniqueName, int version, std::function<QByteArray()> generator)
: mName(std::move(uniqueName))
, mVersion(version)
, mGenerator(std::move(generator))
{
    Q_ASSERT(mName.size() < 32);
    if (!readCache())
    {
        qInfo() << "Calculating new value for"<<mName<<"version"<<version;
        mData = mGenerator();
        writeCache();
    }
}

bool CachedData::readCache()
{
    QString path = cachePath();
    qDebug() << "Checking"<<path<<"for cache of"<<mName<<"version"<<mVersion;
    
    QSettings checksums;
    checksums.beginGroup("cachedFileChecksums");
    QSettings versions;
    versions.beginGroup("cachedFileVersions");
    
    if (!checksums.contains(path) || !versions.contains(path))
    {
        qInfo() << "No cached value found for"<<mName<<"version"<<mVersion;
        return false;
    }
    
    bool const settingsValid =
    checksums.contains(path)
    && versions.contains(path)
    && checksums.value(path).canConvert<int>()
    && versions.value(path).canConvert<int>();
    
    Q_ASSERT(settingsValid);
    
    if (!settingsValid)
    {
        qCritical() << "Settings data about cache is invalid.";
        checksums.remove(path);
        versions.remove(path);
        return false;
    }
    else
    {
        int const version = versions.value(path).value<int>();
        if (version != mVersion)
        {
            qInfo() << "Cached value for"<<mName<<"is out of date.";
            return false;
        }
        
        QFile cacheFile(path);
        if (!cacheFile.open(QIODevice::ReadOnly))
        {
            qWarning() << "Failed to find previously cached value for"<<mName<<"version"<<mVersion;
            return false;
        }
        else
        {
            qDebug() << "Reading cache of"<<mName<<"version"<<mVersion<<"from cache file"<<path;
            QByteArray data = cacheFile.readAll();
            int const checksum = checksums.value(path).value<int>();
            int const dataChecksum = qChecksum(data.constData(), data.size());
            if (checksum != dataChecksum)
            {
                qWarning() << "Cached value for"<<mName<<"version"<<mVersion<<"failed checksum.";
                return false;
            }
            mData = std::move(data);
            return true;
        }
    }
}


void CachedData::writeCache() const
{
    QString path = cachePath();
    // Ensure directory exists
    VERIFY(QFileInfo(path).dir().mkpath("."));
    QFile cacheFile(path);
    if (!cacheFile.open(QIODevice::WriteOnly))
    {
        qWarning() << "Failed to open file to cache value of"<<mName<<"version"<<mVersion<<":"<<path;
    }
    else
    {
        qInfo() << "Writing cache of"<<mName<<"version"<<mVersion<<"to"<<path;
        cacheFile.write(mData);
        cacheFile.close();
        QSettings settings;
        settings.beginGroup("cachedFileChecksums");
        settings.setValue(path, qChecksum(mData.constData(), mData.size()));
        settings.endGroup();
        settings.beginGroup("cachedFileVersions");
        settings.setValue(path, mVersion);
        settings.endGroup();
    }
}

void CachedData::clearCache() const
{
    QString path = cachePath();
    QFile file(path);
    if (file.exists())
    {
        if (!file.remove())
        {
            qWarning() << "Failed to remove cache file"<<path;
        }
    }
    QSettings settings;
    settings.beginGroup("cachedFileChecksums");
    settings.setValue(path, qChecksum(mData.constData(), mData.size()));
    settings.endGroup();
    settings.beginGroup("cachedFileVersions");
    settings.setValue(path, mVersion);
    settings.endGroup();
}


QByteArray CachedData::data() const
{
    return mData;
}

QString CachedData::cachePath() const
{
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QString cleanName = QString::fromUtf8(mName.toUtf8().toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
    return QDir(cacheDir).filePath(QString("cache_")+cleanName);
}
