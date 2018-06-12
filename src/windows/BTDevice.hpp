#ifndef WIN_BTDEVICE_HPP
#define WIN_BTDEVICE_HPP

/**
  ACHTUNG: Reihenfolge der Includes ist WICHTIG!
*/
#include <winsock2.h>
#include <windows.h>
#include <ws2bth.h>
#include <BluetoothAPIs.h>
#include <stdio.h>
#include <algorithm>

#include <QtGlobal>
#include <QUuid>
#include <QMutex>
#include <QTimer>
#include "../config/ProjectConst.hpp"
#include "../bluetooth/ABTDevice.hpp"
#include "BtConnectThread.hpp"
#include "BTDisconnectThread.hpp"
#include "BTDiscoverThread.hpp"
#include "BTCommWorker.hpp"
#include "../spx42/SpxCommandDef.hpp"

namespace spx
{
  /**
   * @brief The WinBTDevice class Die BT Geräteklasse für WINDOWS
   */
  class LocalBTDevice : public ABTDevice
  {
    private:
      Q_OBJECT

      //###########################################################################
      //#### Member Variable                                                   ####
      //###########################################################################
    private:
      SOCKET btSocket;                                     //! der Bluetooth Socket
      bool winsockChecked;                                 //! ist die Winsock 2.2 verfügbar
      WSADATA WSAData;                                     //! Struktur für WSAStartup
      BTDiscoverThread *discoverWorker;                 //! Nebenläufig SPX42 suchen
      BTConnectThread *connectWorker;                   //! Nebenläufig verbinden / paaren
      BTCommWorker *commWorker;                         //! nebenläufig komunizieren
      QByteArray recBuffer;                                //! Puffer für empfangene Daten (reentrant!)

    public:
      LocalBTDevice( Logger *log, const char *dAddr );       //! Logger, MAC-Adressec_string oder NULL
      LocalBTDevice( Logger *log, const QByteArray& dAddr ); //! Logger, MAC-Adressec_string oder NULL
      LocalBTDevice( const char *dName = nullptr );          //! MAC-Adressec_string oder NULL
      LocalBTDevice( const QByteArray& dAddr );              //! MAC-Adressec_string oder NULL
      virtual ~LocalBTDevice();                              //! Destruktor zum aufräumen
      virtual void discoverDevices(void);                  //! Asyncron, finde BT-Geräte, sende Signal bei Finden
      virtual int connectBT( void );                       //! verbinde bei vorhandenem Gerätenamen
      virtual int connectBT( const char *dAddr, const char *pi = nullptr ); //! verbinde zu Addr
      virtual int connectBT( const QByteArray& dAddr, const QByteArray& pi = nullptr );    //!verbinde zu Addr
      virtual int disconnectBT( void );                    //! trenne die Verbindung
      virtual int write( const char *str, int len );       //! schreibe zum verbundenen Gerät
      virtual int write( const QByteArray& cmd );          //! schreibe zum verbundenen Gerät
      virtual int read( char *dest, int maxlen );          //! lese vom verbundenen Gerät
      virtual int avail( void );                           //! vom Gerät bereits übertragene, verfügbare Daten
      virtual bool spxPDUAvail(void);                      //! ist eine SPX42 PDU empfangen
      virtual QByteArray getSpxPDU(void);                  //! gib eine SPX-PDU zurück
      virtual bool removeCrLf(void);                       //! entferne CRLF am Anfang
      virtual bool isConnected();                          //! erfrage ob Verbindung besteht
      virtual QString getErrorText( int errnr, qint64 langId ) const;     //! erfrage Text zu einem Fehler
#if defined(SPX_DEBUGING)
      QByteArray& getRecBuffer(void);                      //! TODO: debuggingfunktion, später wieder entfernen
#endif

    private:
      int initWinsock(void);                               //! Initialisiert WINSOCK2 (nur Windows)
      void startCommWorker();                              //! starte den Kommunikationsworker
      void killThreads(void);                              //! entfernt alle laufenden Threads
      void removeDiscoverWorker(void);                     //! entferne den Discoverthread
      void removeConnectWorker(void);                      //! entferne den ConnectThread
      void removeCommWorker(void);                         //! entferne den kommunikationsthread
      //
    private slots:
      void btDiscoveringSlot(void);                        //! Wenn das discovering beginnt
      void btNewDeviceFoundSlot(BluetoothDeviceDescriber *device ); //! Slot wenn ein neues Gerät gefunden wurde
      void btDiscoverEndSlot(void);                        //! wenn das Discovering beendet wurde
      void connectingSlot(void);                           //! Wenn die Verbindung zum SPX42 aufgebaut wird
      void connectSlot(const QByteArray& dAddr);           //! Wenn die Verbindung hergestellt ist
      void disconnectSlot(void);                           //! Wenn die Verbindung unterbrochen/ beendet wurde
      void connectErrorSlot(int errnr);                    //! Wenn ein Fehler aufgetreten ist
      void btDataRecivedSlot( const char * data, int len );//! Signal, wenn Daten angekommen sind

  };
}

#endif // WIN_BTDEVICE_HPP
