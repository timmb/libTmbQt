#pragma once

#include <QtCore>
#include <functional>

/// Class to either calculate fresh data or read it from
/// the cache directory if it has been previously cached
class CachedData
{
public:
    CachedData(QString uniqueName, int version, std::function<QByteArray()> generatorFunction);
    
    QByteArray data() const;
    void clearCache() const;
    
private:
    /// Cleaned path used for cache file
    QString cachePath() const;
    bool readCache();
    void writeCache() const;
    
    QString mName;
    int mVersion;
    std::function<QByteArray()> mGenerator;
    
    QByteArray mData;
};
