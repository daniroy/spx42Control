#ifndef SPX42CONTROLMAINWIN_HPP
#define SPX42CONTROLMAINWIN_HPP

#include <memory>

#include <QMainWindow>
#include <QFontDatabase>
#include <QMessageBox>
#include <QFont>
#include <QDebug>
#include <QTimer>
#include <QTabWidget>
#include <QCloseEvent>
#include <QStringList>

#include "config/ProjectConst.hpp"
#include "logging/Logger.hpp"
#include "config/AppConfigClass.hpp"
#include "utils/AboutDialog.hpp"
#include "guiFragments/ConnectFragment.hpp"
#include "guiFragments/DeviceConfigFragment.hpp"
#include "guiFragments/GasFragment.hpp"
#include "guiFragments/LogFragment.hpp"
#include "guiFragments/ChartsFragment.hpp"
#include "utils/SPX42Config.hpp"
#include "bluetooth/LocalBTDevice.hpp"

namespace Ui
{
  //
  // Vorwärtsdeklaration, include in der CPP
  //
  class SPX42ControlMainWin;
}

namespace spx
{
  class SPX42ControlMainWin : public QMainWindow
  {
    private:
      Q_OBJECT
      std::unique_ptr<Ui::SPX42ControlMainWin> ui;
      std::shared_ptr<Logger> lg;                               //! Loggerobjekt für Logs
      const std::unique_ptr<QTimer> watchdog;                   //! Wachhund für Timeouts
      const std::shared_ptr<SPX42Config> spx42Config;           //! Konfiguration des verbundenen SPX42
      ApplicationStat currentStatus;                            //! welchen Status hat die App?
      int watchdogTimer;                                        //! Zeitspanne zum Timeout
      AppConfigClass cf;                                        //! Konfiguration aus Datei
      ApplicationTab currentTab;                                //! welcher Tab ist aktiv?
      QStringList tabTitle;                                     //! Tab Titel (nicht statisch, das Objekt gibts eh nur einmal)
      std::shared_ptr<LocalBTDevice> btDevice;                      //! Das Blauzahnobjekt, im Konstruktor zu erzeugen

    public:
      explicit SPX42ControlMainWin(QWidget *parent = 0);
      ~SPX42ControlMainWin();
      void closeEvent(QCloseEvent *event);                      //! Das Beenden-Ereignis

    private:
      bool createLogger();                                      //! Erzeuge den Logger
      void fillTabTitleArray( void );                           //! Fülle das Titelarray lokalisiert
      bool setActionStati( void );                              //! setze Actions entsprchend des Status
      bool connectActions( void );                              //! Verbinde Actions mit Slots
      void createApplicationTabs( void );                       //! Erzeuge die (noch leeren) Tabs
      void clearApplicationTabs( void );                        //! Leere die Tabs
      void simulateLicenseChanged( LicenseType lType );         //! Simuliere lizenzwechsel
      ApplicationTab getApplicationTab( void );                 //! Welcher Tab war noch aktiv?

    private slots:
      void aboutActionSlot( bool checked );                     //! ABOUT wurde gefordert
      void quitActionSlot( bool checked );                      //! ENDE wurde gefordert
      void tabCurrentChangedSlot( int idx );                    //! TAB Index gewechselt
      void licenseChangedSlot( void );                          //! Lizenztyp getriggert
      void simulateIndividualLicenseChanged( void );            //! Individuallizenz geändert
      // Blutooth Slots vom BTDevice
      /*
      void btConnectingSlot( void );                            //! Signal, wenn eine Verbindung aufgebaut wird
      void btConnectedSlot( const QByteArray& dAddr );          //! Signal, wenn eine Verbindung zustande gekommen ist
      void btDisconnectSlot( void );                            //! Signal, wenn eine Verbindung beendet/unterbrochen wurde
      void btConnectErrorSlot( int errnr );                     //! Signal, wenn es Fehler beim Verbinden gab
      void btDataRecivedSlot();                                 //! Signal, wenn Daten kamen
      void btPairingPinRequestSlot(void);                       //! Signal, wenn Pairing gefordert wird
      */
  };
} // namespace spx42


/*
      ==BtDevice....==
      void btConnectingSig( void );                               //! Signal, wenn eine Verbindung aufgebaut wird
      void btConnectedSig( const QByteArray& dAddr );             //! Signal, wenn eine Verbindung zustande gekommen ist
      void btDisconnectSig( void );                               //! Signal, wenn eine Verbindung beendet/unterbrochen wurde
      void btConnectErrorSig( int errnr );                        //! Signal, wenn es Fehler beim Verbinden gab
      void btDataRecivedSig();                                    //! Signal, wenn Daten kamen
      void btPairingPinRequestSig(void);                          //! Signal, wenn Pairing gefordert wird

 */
#endif // SPX42CONTROLMAINWIN_HPP
