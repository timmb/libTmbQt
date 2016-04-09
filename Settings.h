#pragma once
#include <QtCore>

class Settings : public QObject
{
	Q_OBJECT;

public:
	/// Will verify types are correct and refuse to load any settings that do not have a default set.
	/// \param backupFile will be loaded if jsonFilePath fails to load or parse. Can be "" for no backup
	Settings(QString jsonFilePath, QString backupJsonFilePath, QMap<QString, QVariant> defaultValues, QObject*);

	/// Will save settings to file
	virtual ~Settings();

	bool hasValue(QString const& key) const;
	QVariant value(QString const& key) const;
	virtual void setValue(QString const& key, QVariant value);
	void insert(QString const& key, QVariant const& value)
	{
		setValue(key, value);
	}
	QStringList keys() const;
	QVariant defaultValue(QString const& key) const;
	///// Trigger callback for updating a value without changing the value itself (and so avoiding
	///// an autosave of settings).
	//void triggerValueChangedSignal(QString const& key) const;

	void setSaveFile(QString filePath);
	QString saveFile() const { return mSaveFile; }


	/// Save to file in destructor. Default is true.
	bool saveOnDestruction;
	/// Save to file whenever a value is changed. Default is true.
	bool saveOnSet;

public Q_SLOTS:
	void loadFromFile();
	void saveToFile();
	void saveToBackupFile();

Q_SIGNALS:
	void valueChanged(QString const& key, QVariant const& newValue) const;
	void saveSucceeded(QString const& saveFile);
	void saveFailed(QString const& saveFile);

protected:
	/// Any settings that have incorrect type will be converted, or if that is not possible, replaced with
	/// a default value. Conversion/replacement will trigger a qWarning message. Missing values will be
	/// inserted
	void validateTypes();


private:
	void saveToFile(QString const& saveFilePath);
	bool loadFromFile(QString const& jsonSettingsFile);

	bool readJsonFile(QString const& jsonSettingsFile, QJsonObject & out_json);

	QString mSaveFile;
	QString mBackupFile;
	QVariantMap mSettings;
	QVariantMap const mDEFAULTS;
};


