#ifndef SRC_LOGGING_LOGGER_HPP_
#define SRC_LOGGING_LOGGER_HPP_
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QString>
#include <QTextStream>
#include <QtGlobal>
#include <memory>
#include <string>

enum LgThreshold : quint8
{
  LG_NONE = 0,
  LG_CRIT,
  LG_WARN,
  LG_INFO,
  LG_DEBUG
};

class Logger
{
private:
  LgThreshold threshold;                   //! Logstatus
  std::unique_ptr<QFile> logFile;          //! Zeiger auf das Logdateiobjekt
  std::unique_ptr<QTextStream> textStream; //! Zeiger auf einen Textstrom
  QDateTime dateTime;                      //! das lokale Datum/Zeit objekt
  static const QString dateTimeFormat;     //! Format der Zeitausgabe
  static const QString DEBUG_STR;          //! String für Debuglevel
  static const QString INFO_STR;           //! String für Infolevel
  static const QString WARN_STR;           //! String für Warnlevel
  static const QString CRIT_STR;           //! String für Kritischen level

public:
  Logger();          //! Konstruktor mit Zeiger auf das Konfig-Objekt
  virtual ~Logger(); //! Destruktor
  int startLogging(LgThreshold th = LG_DEBUG,
                   QString fn = "logging.log"); //! Loggen beginnen
  void setThreshold(LgThreshold th);            //! Setzte Loggingstufe
  LgThreshold getThreshold(void);               //! lese Loggingstufe
  //
  void info(const QString& msg);     //! INFO Ausgaben
  void info(const std::string& msg); //! INFO Ausgaben
  void info(const char* msg);        //! INFO Ausgaben
  //
  void debug(const QString& msg);     //! DEBUG Ausgaben
  void debug(const std::string& msg); //! DEBUG Ausgaben
  void debug(const char* msg);        //! DEBUG Ausgaben
  //
  void warn(const QString& msg);     //! WARN Ausgaben
  void warn(const std::string& msg); //! WARN Ausgaben
  void warn(const char* msg);        //! WARN Ausgaben
  //
  void crit(const QString& msg);     //! CRIT Ausgaben
  void crit(const std::string& msg); //! CRIT Ausgaben
  void crit(const char* msg);        //! CRIT Ausgaben
  //
  void shutdown(); //! Logger explizit herunterfahren

private:
  QString getDateString(); //! interne Funktion für den Datumsstring
};

#endif /* SRC_LOGGING_LOGGER_HPP_ */
