﻿#ifndef LOGGERCLASS_HPP
#define LOGGERCLASS_HPP

#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <QVector>
#include "CurrBuildDef.hpp"

namespace spx
{
  class AppConfigClass
  {
    private:
    static const QString constBuildDate;           //! Builddatum
    static const QString constBuildNumStr;         //! Buildnummer
    static const QString constLogGroupName;        //! Gruppenname Logeinstellungen
    static const QString constLogFileKey;          //! Einstellung für Logdatei
    static const QString constNoData;              //! Kennzeichner für keine Daten
    static const QString constAppGroupName;        //! Gruppenname für App Einstellungen
    static const QString constAppTimeoutKey;       //! Einstellung für Watchdog
    static const QString constAppDatabaseNameKey;  //! Einstellungen für Datenbankdatei
    static const QString constAppDatabasePathKey;  //! An welchem Pfad fidne ich die Datenbank
    static const int defaultWatchdogTimerVal;      //! Default für Watchdog
    static const QString constAppThresholdKey;     //! Einstellung für Logebene
    static const qint8 defaultAppThreshold;        //! defaultwert für Loggingebene
    static const QString defaultDatabaseName;      //! defaultwert für den Namen der Datenbank
    static const QString defaultDatabasePath;      //! der Standartpfad für die Database (OS abhängig)
    // ab hier die Konfiguration lagern
    QString configFile;    //! wie nennt sich die Konfigurationsdatei
    QString logfileName;   //! Wie heisst das Logfile
    int watchdogTimer;     //! welchen Wert hat der Timer
    qint8 logThreshold;    //! welche Loggerstufe hat die App
    QString databaseName;  //! wie ist der name der Datenbank
    QString databasePath;  //! an welchem Pfad finde ich die Datenbank

    public:
    AppConfigClass( void );                         //! Konstruktor
    virtual ~AppConfigClass();                      //! Destruktor
    bool loadSettings( void );                      //! lade Einstellungen aus default Konfigdatei
    bool loadSettings( QString &configFile );       //! lade Einstellungen aus benannter Konfigdatei
    bool saveSettings( void );                      //! sichere Einstellungen
    QString getConfigFile( void ) const;            //! Name der Konfigdatei ausgeben
    QString getLogfileName( void ) const;           //! Name der Logdatei ausgeben
    void setLogfileName( const QString &value );    //! Name der Logdatei setzten
    int getWatchdogTime( void );                    //! Wert des Watchdog holen
    void setLogThreshold( qint8 th );               //! setzte Loggingstufe in Config
    qint8 getLogTreshold( void );                   //! hole Loggingstufe aus config
    static QString getBuildDate();                  //! hole das Builddatum als String
    static QString getBuildNumStr();                //! hole die Buildnummer als String
    QString getDatabaseName() const;                //! datenbankdateiname
    void setDatabaseName( const QString &value );   //! datenbankdatei Name
    QString getDatabasePath() const;                //! auf welchem Datenpfad finde ich die Datenbank
    void setDatabasePath( const QString &value );   //! auf welchem Datenpfad finde ich die Datenbank
    QString getFullDatabaseLocation( void ) const;  //! volle Beschreibung des Ortes + Name der Datenbank

    private:
    // Logeinstellungen
    bool loadLogSettings( QSettings &settings );
    void makeDefaultLogSettings( QSettings &settings );
    bool saveLogSettings( QSettings &settings );
    // allg. Programmeinstellungen
    bool loadAppSettings( QSettings &settings );
    void makeAppDefaultSettings( QSettings &settings );
    bool saveAppSettings( QSettings &settings );
  };
}
#endif  // LOGGERCLASS_HPP
