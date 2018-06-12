#ifndef BTDEVICE_HPP
#define BTDEVICE_HPP

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QBluetoothUuid>
#include <QBluetoothServiceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QBluetoothLocalDevice>
#include "../config/ProjectConst.hpp"
#include "../logging/Logger.hpp"


namespace spx
{
  class LocalBTDevice : public QObject
  {
    private:
      Q_OBJECT
      //###########################################################################
      //#### Member Variable                                                   ####
      //###########################################################################
    protected:
      QList<QBluetoothHostInfo> localAdapters;
      IndicatorStati isInitOk;
      Logger *lg;
      QBluetoothServiceDiscoveryAgent discoveryAgent;

    public:
      explicit LocalBTDevice( Logger *log );                      //! Logger, MAC-Adressec_string oder NULL
      ~LocalBTDevice();                                           //! Destruktor, natürlich
      int setLogger( Logger * log );                              //! den Systemlogger übergeben
      void discoverDevices(const QBluetoothUuid &uuid);           //! Asyncron, finde BT-Geräte, filtgere service id, sende Signal bei Finden
      void stopDiscoverDevices(void);                             //! Stoppe discovering
      QBluetoothHostInfo getBtHostInfo( int adapterIndex = 0 );   //! Hostinfo des Adapters adapterIndex (oder 0)
      int connectBT( void ) = 0;                                  //! verbinden, wenn Gerätename vorhanden
      int connectBT( const char *dAddr, const char *pin = nullptr ) = 0;                   //! verbinden mit Adresse/Name und PIN
      int connectBT( const QByteArray& dAddr, const QByteArray& pin = nullptr ) = 0;       //! verbinden mit Adresse/Name und PIN
      int disconnectBT( void ) = 0;                               //! trenne die Verbindung zum Gerät
      const QByteArray& getConnectedDeviceAddr(void);             //! gib die Adresse des verbundenen Gerätes zurück
      int write( const char *str, int len ) = 0;                  //! schreibe zum Gerät (wenn verbunden)
      int write( const QByteArray& cmd ) = 0;                     //! schreibe zum Gerät (wenn verbunden)
      int read( char *dest, int maxlen ) = 0;                     //! lese vom Gerät (wenn verbunden)
      int avail( void ) = 0;                                      //! vom Gerät bereits übertragene, verfügbare Daten
      bool spxPDUAvail(void) = 0;                                 //! ist eine SPX42 PDU empfangen
      QByteArray getSpxPDU(void) = 0;                             //! gib eine SPX-PDU zurück
      bool removeCrLf(void) = 0;                                  //! entferne CRLF am Anfang
      bool isConnected() = 0;                                     //! Info, ob das Gerät verbunden ist
      QString getErrorText( int errnr, qint64 langId ) const = 0; //! Liefere Erklärung für Fehler, in sprache langId
      //
    private slots:
      void serviceDiscovered(const QBluetoothServiceInfo &serviceInfo);
      void discoveryFinished();
      // void onRemoteDevicesItemActivated(QListWidgetItem *item);
      //
    signals:
      void btDiscoveringSig(void);                                //! Signal, wenn discovering startet
      void btDeviceDiscoveredSig(const QBluetoothServiceInfo &serviceInfo ); //! Signal, wenn ein SPX42 gefunden wurde
      void btDiscoverEndSig(void);                                //! Signal, wenn discovering beendet ist
      void btConnectingSig( void );                               //! Signal, wenn eine Verbindung aufgebaut wird
      void btConnectedSig( const QByteArray& dAddr );             //! Signal, wenn eine Verbindung zustande gekommen ist
      void btDisconnectSig( void );                               //! Signal, wenn eine Verbindung beendet/unterbrochen wurde
      void btConnectErrorSig( int errnr );                        //! Signal, wenn es Fehler beim Verbinden gab
      void btDataRecivedSig();                                    //! Signal, wenn Daten kamen
      void btPairingPinRequestSig(void);                          //! Signal, wenn Pairing gefordert wird

  };
}

#endif // BTDEVICE_HPP
