#include "jsonsettingsfile.h"
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QStandardPaths>
#include <QDebug>
#include <QSaveFile>
#include <QFileInfo>

class JsonSettingsFilePrivate {

  JsonSettingsFilePrivate(JsonSettingsFile *t_qPtr) : q_ptr(t_qPtr) {}

  JsonSettingsFile *q_ptr;
  QString m_settingsFilePath;

  QJsonObject m_dataHolder;

  Q_DECLARE_PUBLIC(JsonSettingsFile)
};



JsonSettingsFile::JsonSettingsFile(QQuickItem *t_parent) :
  QQuickItem(t_parent),
  d_ptr(new JsonSettingsFilePrivate(this))
{
}

JsonSettingsFile *JsonSettingsFile::getInstance()
{
  if(s_globalSettings==0)
  {
    s_globalSettings = new JsonSettingsFile();
  }
  return s_globalSettings;
}

bool JsonSettingsFile::fileExists(const QString &t_filePath) const
{
  bool retVal = false;
  QFile f;
  f.setFileName(t_filePath);
  if(f.exists())
  {
    retVal = true;
  }
  return retVal;
}

void JsonSettingsFile::reloadFile()
{
  Q_D(JsonSettingsFile);
  loadFromFile(d->m_settingsFilePath);
}

bool JsonSettingsFile::loadFromStandardLocation(const QString &t_fileName)
{
  qDebug() << "[json-settings-qml] Attempting to load settings file from standard location:" << QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)).arg(t_fileName);
  return loadFromFile(QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)).arg(t_fileName));
}

bool JsonSettingsFile::loadFromFile(const QString &t_filePath)
{
  Q_D(JsonSettingsFile);
  bool retVal = false;
  QFile settingsFile;
  settingsFile.setFileName(t_filePath);
  if(settingsFile.exists() && settingsFile.open(QFile::ReadOnly))
  {
    d->m_settingsFilePath = t_filePath;
    QJsonParseError err;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(settingsFile.readAll(), &err);
    if(err.error == QJsonParseError::NoError)
    {
      d->m_dataHolder = jsonDoc.object();
      retVal = true;
      emit settingsChanged(this);
      qDebug() << "[json-settings-qml] Settings file loaded:" << t_filePath;
    }
    else
    {
      qWarning() << "[json-settings-qml] Error reading settings file:" << err.errorString();
    }
    settingsFile.close();
  }
  else
  {
    qWarning() << "[json-settings-qml] Settings file does not exists:" << t_filePath;
  }
  return retVal;
}

void JsonSettingsFile::saveToFile(const QString &t_filePath, bool t_overwrite)
{
  Q_D(JsonSettingsFile);
  QFileInfo fInfo;
  QSaveFile settingsFile;
  fInfo.setFile(t_filePath);
  settingsFile.setFileName(t_filePath);
  if((t_filePath.isEmpty() == false) && (!fInfo.exists() || t_overwrite) && settingsFile.open(QFile::WriteOnly))
  {
    QJsonDocument jsonDoc;
    jsonDoc.setObject(d->m_dataHolder);
    settingsFile.write(jsonDoc.toJson());
    settingsFile.commit();
  }
}

QString JsonSettingsFile::getCurrentFilePath()
{
  Q_D(JsonSettingsFile);
  return d->m_settingsFilePath;
}

bool JsonSettingsFile::hasOption(const QString &t_key)
{
  Q_D(JsonSettingsFile);
  bool retVal=false;
  if(d->m_dataHolder.value(t_key) != QJsonValue::Undefined)
  {
    retVal = true;
  }
  return retVal;
}

QString JsonSettingsFile::getOption(const QString &t_key)
{
  Q_D(JsonSettingsFile);
  QString retVal;
  if(hasOption(t_key))
  {
    retVal = d->m_dataHolder.value(t_key).toString();
  }
  else
  {
    if(d->m_settingsFilePath.isEmpty() == false)
    {
      qWarning() << "[json-settings-qml] Could not find data for key:" << t_key;
    }
  }
  return retVal;
}

bool JsonSettingsFile::setOption(const QString &t_key, const QString &t_value, bool t_addIfNotExists)
{
  Q_D(JsonSettingsFile);
  bool retVal = false;
  if(hasOption(t_key) || t_addIfNotExists)
  {
    d->m_dataHolder.insert(t_key, t_value);
    retVal=true;
    emit settingsChanged(this);
  }
  else
  {
    qWarning() << "[json-settings-qml] Refused to set nonexisting key:" << t_key;
  }

  return retVal;
}

bool JsonSettingsFile::dropOption(const QString &t_key)
{
  Q_D(JsonSettingsFile);
  bool retVal = false;
  if(hasOption(t_key))
  {
    d->m_dataHolder.remove(t_key);
    retVal = true;
    emit settingsChanged(this);
  }
  else
  {
    qDebug() << "[json-settings-qml] Refused to delete nonexistant key:" << t_key;
  }

  return retVal;
}

JsonSettingsFile *JsonSettingsFile::s_globalSettings = 0;
