#include "Settings.h"




Settings::Settings(QString jsonFilePath, QString backupJsonPath, QMap<QString, QVariant> defaultValues, QObject* parent)
	: QObject(parent)
	, saveOnDestruction(false)
	, saveOnSet(true)
	, mSaveFile(std::move(jsonFilePath))
	, mBackupFile(std::move(backupJsonPath))
	, mDEFAULTS(std::move(defaultValues))
{
	if (!mBackupFile.isEmpty())
	{
		QJsonObject json;
		bool const doesBackupLoadOk = readJsonFile(mBackupFile, json);
		if (!doesBackupLoadOk)
		{
			qCritical() << "Unable to read backup settings file" << mBackupFile << "- please save new backup.";
		}
	}
	loadFromFile();
	validateTypes();
}

Settings::~Settings()
{
	if (saveOnDestruction)
	{
		saveToFile();
	}
}

void Settings::validateTypes()
{
	for (QString const& key : mSettings.keys())
	{
		if (!mDEFAULTS.contains(key))
		{
			qWarning() << "Setting with unrecognised key of" << key << "loaded. Could be from a previous version. Removing this setting.";
			mSettings.remove(key);
		}
	}

	// Check all values have correct type
	for (QString const& key : mDEFAULTS.keys())
	{
		QVariant const& defaultValue = mDEFAULTS.value(key);
		auto correctType = defaultValue.type();
		if (mSettings.contains(key))
		{
			QVariant value = mSettings.value(key);
			auto valueType = value.type();
			if (valueType != correctType)
			{
				if (valueType == (size_t)QMetaType::QVariantList && correctType == (size_t)QMetaType::QJsonArray)
				{
					mSettings.insert(key, QJsonArray::fromVariantList(value.toList()));
					Q_ASSERT(mSettings.value(key).type() == correctType);
				}
				else if (value.canConvert(correctType) && value.convert(correctType))
				{
					mSettings.insert(key, value);
					Q_ASSERT(mSettings.value(key).type() == correctType);
				}
				else
				{
					qWarning() << "Failed to understand loaded value for" << key << ". Expected type" << QMetaType::typeName(correctType)
						<< "but read in \"" << qPrintable(QMetaType::typeName(valueType)) << qPrintable("\" from JSON. Reverting to default value.");
					mSettings.insert(key, defaultValue);
					Q_ASSERT(mSettings.value(key).type() == correctType);
				}
			}
		}
		else
		{
			qWarning() << "No value found for" << key << "- using default value:" << defaultValue;
			mSettings.insert(key, defaultValue);
			Q_ASSERT(mSettings.value(key).type() == correctType);
		}
	}
}


void Settings::setSaveFile(QString filePath)
{
	mSaveFile = std::move(filePath);
}

bool Settings::hasValue(QString const& key) const
{
	return mSettings.contains(key);
}

QVariant Settings::value(QString const& key) const
{
	if (!mSettings.contains(key))
	{
		qWarning() << "Settings: no value found for key:" << key;
		return QVariant();
	}
	return mSettings.value(key);
}

void Settings::setValue(QString const& key, QVariant value)
{
	QVariant::Type targetType = value.type();

	if (mDEFAULTS.count(key) == 1)
	{
		targetType = mDEFAULTS.value(key).type();
	}
	else
	{
		qWarning() << "Setting value for which no default is defined:" << key << QString("(%1)").arg(value.typeName());
		//Q_ASSERT(mDEFAULTS.count(key) == 1);
	}

	if (value.convert(targetType))
	{
		bool const isValueNew = !mSettings.contains(key) || mSettings.value(key) != value;
		mSettings.insert(key, value);
		Q_EMIT valueChanged(key, value);
		if (saveOnSet && isValueNew)
		{
			saveToFile();
		}
	}
	else
	{
		qCritical() << "Attempt to set" << key << "with invalid type (cannot convert" << value.typeName() << "to" << QVariant(targetType).typeName()
			<<"- ignoring request.";
		Q_ASSERT(mDEFAULTS.count(key) == 0 || value.canConvert(mDEFAULTS.value(key).type()));
	}
}

QStringList Settings::keys() const
{
	return mSettings.keys();
}



QVariant Settings::defaultValue(QString const& key) const
{
	return mDEFAULTS.value(key, QVariant(QVariant::Invalid));
}

//void Settings::triggerValueChangedSignal(QString const& key) const
//{
//	if (!mSettings.contains(key))
//	{
//		qWarning() << "No value found for key:" << key;
//	}
//	else
//	{
//		Q_EMIT valueChanged(key, mSettings.value(key));
//	}
//}

void Settings::loadFromFile()
{
	bool const wasLoadSuccessful = loadFromFile(mSaveFile);
	if (!wasLoadSuccessful)
	{
		if (!mBackupFile.isEmpty())
		{
			qCritical() << "Settings failed to load from" << mSaveFile << "- will attempt to load backup from" << mBackupFile;
			bool const wasBackupLoadSuccessful = loadFromFile(mBackupFile);
			if (wasBackupLoadSuccessful)
			{
				qInfo() << "Replacing" << mSaveFile << "with loaded backup settings.";
				saveToFile();
			}
			else
			{
				qCritical() << "Also failed to load settings from backup file" << mBackupFile << "- default values will be used.";
			}
		}
	}
}

bool Settings::loadFromFile(QString const& jsonSettingsFile)
{
	QJsonObject json;
	bool isJsonReadOk = readJsonFile(jsonSettingsFile, json);

	if (isJsonReadOk)
	{
		mSettings = json.toVariantMap();
		for (QString const& key : mSettings.keys())
		{
			qDebug() << "Loaded setting" << key << "as" << mSettings.value(key) << "from" << jsonSettingsFile;
		}
		validateTypes();
		for (QString const& key : mDEFAULTS.keys())
		{
			if (!json.contains(key))
			{
				qWarning() << "No value found for" << key << "when loading" << jsonSettingsFile << "- retaining previous value of" << mSettings.value(key);
			}
		}
		qInfo() << "Successfully read settings from" << jsonSettingsFile;
		return true;
	}
	else
	{
		return false;
	}
}

bool Settings::readJsonFile(QString const& jsonSettingsFile, QJsonObject & out_json)
{
	QFile loadFile(jsonSettingsFile);
	QString loadFilePath = QFileInfo(loadFile).absoluteFilePath();
	if (!QFileInfo(loadFile).exists())
	{
		qWarning() << "Failed to find settings file:" << loadFilePath;
		return false;
	}
	if (!loadFile.open(QFile::ReadOnly))
	{
		qWarning() << "Failed to open settings file:" << loadFilePath;
		return false;
	}
	QJsonParseError error;
	QByteArray data = loadFile.readAll();
	if (data.isEmpty())
	{
		qWarning() << "Failed to read any data from" << loadFilePath;
		return false;
	}
	QJsonDocument jDoc = QJsonDocument::fromJson(data, &error);
	if (error.error != QJsonParseError::NoError)
	{
		qCritical() << "Failed to parse JSON in" << loadFilePath << " at character" << error.offset << ":"
			<< error.errorString();
		return false;
	}
	if (!jDoc.isObject())
	{
		qCritical() << "Failed to parse JSON as it is not a JSON object (e.g. {key: value})";
		return false;
	}
	out_json = jDoc.object();
	return true;
}

void Settings::saveToFile()
{
	saveToFile(mSaveFile);
}

void Settings::saveToFile(QString const& saveFilePath)
{
	QJsonObject jSettings;
	for (QString const& key : mSettings.keys())
	{
		jSettings[key] = QJsonValue::fromVariant(mSettings.value(key));
	}
	QJsonDocument jDoc(jSettings);
	QFile saveFile(saveFilePath);
	int retCode(-1);
	if (saveFile.open(QFile::WriteOnly))
	{
		retCode = saveFile.write(jDoc.toJson(QJsonDocument::Indented));
	}
	if (retCode < 1)
	{
		qCritical() << "Failed to write settings to" << QFileInfo(saveFile).absoluteFilePath();
		Q_EMIT saveFailed(saveFilePath);
	}
	else
	{
		qDebug() << "Saved settings to" << QFileInfo(saveFile).absoluteFilePath();
		Q_EMIT saveSucceeded(saveFilePath);
	}
}

void Settings::saveToBackupFile()
{
	if (!mBackupFile.isEmpty())
	{
		saveToFile(mBackupFile);
	}
	else
	{
		qInfo() << "Not saving backup file to" << mSaveFile << "as none is set.";
	}
}

