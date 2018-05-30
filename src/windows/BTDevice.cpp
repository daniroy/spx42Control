﻿
#include "../windows/BTDevice.hpp"

namespace spx
{
  BTDevice::BTDevice(Logger *log, const char *dAddr ) :
    ABTDevice( log, dAddr ),
    btSocket( INVALID_SOCKET ),
    winsockChecked(false),
    discoverWorker( nullptr ),
    connectWorker( nullptr ),
    commWorker( nullptr )
  {
    initWinsock();
  }

  BTDevice::BTDevice(Logger *log, const QByteArray& dAddr ) :
    ABTDevice( log, dAddr ),
    btSocket( INVALID_SOCKET ),
    winsockChecked(false),
    discoverWorker( nullptr ),
    connectWorker( nullptr ),
  commWorker( nullptr )
  {
    initWinsock();
  }

  BTDevice::BTDevice(const char *dAddr ) :
    ABTDevice( dAddr ),
    btSocket( INVALID_SOCKET ),
    winsockChecked(false),
    discoverWorker( nullptr ),
    connectWorker( nullptr ),
    commWorker( nullptr )
  {
    initWinsock();
  }

  BTDevice::BTDevice(const QByteArray& dAddr) :
    ABTDevice( dAddr ),
    btSocket( INVALID_SOCKET ),
    winsockChecked(false),
    discoverWorker( nullptr ),
    connectWorker( nullptr ),
    commWorker( nullptr )
  {
    initWinsock();
  }

  BTDevice::~BTDevice()
  {
    if( winsockChecked )
    {
      ::WSACleanup();
    }
    killThreads();
  }

  int BTDevice::initWinsock(void)
  {
    ZeroMemory( &WSAData, sizeof( WSAData ) );
    if( ProjectConst::CXN_SUCCESS != ::WSAStartup( MAKEWORD( 2, 2 ), &WSAData ))
    {
      lg->crit("WinBTDevice::initWinsock -> can't init winsock 2.2 ");
      return( -1 );
    }
    else
    {
      //
      // vermerke das im Objekt
      //
      winsockChecked = true;
    }
    lg->info( "WinBTDevice::initWinsock ->  check winsowck 2.2...OK" );
    return( 0 );
  }

  void BTDevice::startCommWorker()
  {
    killThreads();
    commWorker = new BTCommWorker(this, lg, btSocket);
    connect( commWorker, &BTCommWorker::btConnectErrorSig, this, &BTDevice::connectErrorSlot, Qt::DirectConnection );
    connect( commWorker, &BTCommWorker::btDisconnectSig, this, &BTDevice::disconnectSlot, Qt::DirectConnection );
    connect( commWorker, &BTCommWorker::btDataRecivedSig, this, &BTDevice::btDataRecivedSlot, Qt::DirectConnection );
    commWorker->start();
  }

  void BTDevice::discoverDevices(void)
  {
    //
    // erst mal alles abschiessen
    //
    killThreads();
    lg->debug("WinBTDevice::discoverDevices....");
    discoverWorker  = new BTDiscoverThread( this, lg );
    lg->debug("WinBTDevice::discoverDevices -> connect signals...");
    connect( discoverWorker, &BTDiscoverThread::btDiscoveringSig, this, &BTDevice::btDiscoveringSlot, Qt::DirectConnection );
    connect( discoverWorker, &BTDiscoverThread::newDeviceFoundSig, this, &BTDevice::btNewDeviceFoundSlot, Qt::DirectConnection );
    connect( discoverWorker, &BTDiscoverThread::btDiscoverEndSig, this, &BTDevice::btDiscoverEndSlot );
    connect( discoverWorker, &BTDiscoverThread::btConnectErrorSig, this, &BTDevice::connectErrorSlot );
    lg->debug("WinBTDevice::discoverDevices -> startThread...");
    discoverWorker->start();
  }

  /**
   * Verbinde mit dem BT Gerät, falls initialisierung erfolgreich
   */
  int BTDevice::connectBT( void )
  {
    if( ! winsockChecked )
    {
      lg->crit("WinBTDevice::connectBt -> winsock2 not initialized!");
      return( -1  );
    }
    //
    // erst mal alles abschiessen
    //
    killThreads();
    //
    int lastError = ProjectConst::CXN_SUCCESS;
    //
    // einen BT Socket (wie Netzwerksocket) erzeugen
    //
    btSocket = ::socket( AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM );
    if( INVALID_SOCKET == btSocket )
    {
      lastError = WSAGetLastError();
      lg->crit(QString("WinBTDevice::connectBt -> create socket failed, errcode: <%1>").arg(lastError, 0, 8, QChar('0')));
      isInitOk = IndicatorStati::AERROR;
    }
    if( lastError == ProjectConst::CXN_SUCCESS )
    {
      //
      // den ConnectThread erzeugen und starten
      //
      connectWorker = new BTConnectThread( this, lg, deviceAddr, pin, btSocket );
      // Signale verbinden
      connect( connectWorker, &BTConnectThread::btConnectingSig, this, &BTDevice::connectingSlot, Qt::DirectConnection );
      connect( connectWorker, &BTConnectThread::btConnectedSig, this, &BTDevice::connectSlot, Qt::DirectConnection );
      connect( connectWorker, &BTConnectThread::btDisconnectSig, this, &BTDevice::disconnectSlot, Qt::DirectConnection );
      connect( connectWorker, &BTConnectThread::btConnectErrorSig, this, &BTDevice::connectErrorSlot, Qt::DirectConnection );
      //
      // Thread starten
      //
      connectWorker->start();
      return(0);
    }
    return( -1 );
  }

  int BTDevice::connectBT(const char *dAddr , const char *pi )
  {
    deviceAddr = QByteArray( dAddr );
    if( pi != nullptr )
    {
      pin = QByteArray(pi);
    }
    return(connectBT());
  }

  int BTDevice::connectBT(const QByteArray& dAddr , const QByteArray& pi)
  {
    deviceAddr = QByteArray( dAddr );
    if( pi != nullptr )
    {
      pin = QByteArray(pi);
    }
    return(connectBT());
  }

  int BTDevice::write( const char *str, int len )
  {
    if( commWorker != nullptr )
    {
      return(commWorker->write( str, len ));
    }
    return( 0 );
  }

  int BTDevice::write( const QByteArray& cmd )
  {
    if( commWorker != nullptr )
    {
      return(commWorker->write( cmd ));
    }
    return( 0 );
  }

  /**
   * @brief WinBTDevice::read
   * @param dest
   * @param maxlen
   * @return
   */
  int BTDevice::read( char *dest, int maxlen )
  {
    int cplen;
    //
    if( recBuffer.isEmpty() )
    {
      return( 0 );
    }
    cplen = min( maxlen, recBuffer.length());
    memcpy(dest, recBuffer.constData(), cplen );
    recBuffer.remove(0, cplen);
    return( cplen );
  }

  int BTDevice::avail( void )
  {
    return( recBuffer.length() );
  }

  bool BTDevice::spxPDUAvail(void)
  {
    //
    // erfrage ob es einen Anfang und ein Ende einer SPX-PDU gibt
    //
    int startOfPDU = recBuffer.indexOf(SpxCommandDef::STX);
    int endOfPDU = recBuffer.indexOf(SpxCommandDef::ETX);
    //
    // Auswertung
    //
    if( startOfPDU == -1 || endOfPDU == -1 )
    {
      // noch nichts da!
      return( false );
    }
    if( startOfPDU > endOfPDU )
    {
      // Da ist was richtig schief gegangen!
      // Entferne alles vor STX
      recBuffer.remove(0, startOfPDU-1);
      return( false );
    }
    if( startOfPDU < endOfPDU )
    {
      // da ist etwas!
      return( true );
    }
    return( false );
  }

  bool BTDevice::removeCrLf(void)
  {
    int crChar, lfChar;
    bool wasRemove;

    wasRemove = false;
    crChar = recBuffer.indexOf(SpxCommandDef::CR);
    lfChar = recBuffer.indexOf(SpxCommandDef::LF);
    //
    // solange eines der Zeichen an erster Stelle vorhanden ist
    while( crChar == 0 || lfChar == 0 )
    {
      wasRemove = true;
      recBuffer.remove(0, 1);
      crChar = recBuffer.indexOf(SpxCommandDef::CR);
      lfChar = recBuffer.indexOf(SpxCommandDef::LF);
    }
    return( wasRemove );
  }

  QByteArray BTDevice::getSpxPDU(void)
  {
    //
    // erfrage ob es einen Anfang und ein Ende einer SPX-PDU gibt
    //
    int startOfPDU = recBuffer.indexOf(SpxCommandDef::STX);
    int endOfPDU = recBuffer.indexOf(SpxCommandDef::ETX);
    //
    // Auswertung
    //
    if( startOfPDU == -1 || endOfPDU == -1 )
    {
      // noch nichts da!
      return( nullptr );
    }
    if( startOfPDU > endOfPDU )
    {
      // Da ist was richtig schief gegangen!
      // Entferne alles vor STX
      recBuffer.remove(0, startOfPDU-1);
      return( nullptr );
    }
    if( startOfPDU < endOfPDU )
    {
      // da ist etwas!
      recBuffer.remove(0, startOfPDU+1 );          // STX ist gleich mit weg
      endOfPDU = recBuffer.indexOf(SpxCommandDef::ETX);
      int cplen = endOfPDU;                        // STX und ETX weg
      char dest[ProjectConst::BUFFER_LEN];
      memcpy(dest, recBuffer.constData(), cplen ); // ohne ETX kopieren
      recBuffer.remove(0, cplen+1);                // ETX mit entfernen
      removeCrLf();                                // hintenan CRLF entfernen
      return( QByteArray(dest, cplen ) );
    }
    return( nullptr );
  }

  /**
   * Trenne Verbindung und entferne Socket wenn vorhanden, ansonsten tue NIX
   */
  int BTDevice::disconnectBT( void )
  {
    lg->debug( "WinBTDevice::disconnectBT..." );
    if( isInitOk != IndicatorStati::AERROR && btSocket != INVALID_SOCKET )
    {
      //
      // Thread machen um nicht die GUI zu blockieren
      // Thread hier lokal sichtbar lassen, er löscht sich später selber mit deleteLater()
      //
      BTDisconnectThread *disconnectWorker = new BTDisconnectThread( this, lg, btSocket );
      connect( disconnectWorker, &BTDisconnectThread::btDisconnectSig, this, &BTDevice::disconnectSlot, Qt::DirectConnection );
      connect( disconnectWorker, &BTDisconnectThread::btConnectErrorSig, this, &BTDevice::connectErrorSlot, Qt::DirectConnection );
      //
      // Thread starten
      //
      disconnectWorker->start();
    }
    return( ProjectConst::CXN_SUCCESS );
  }

  bool BTDevice::isConnected()
  {
    if( commWorker != nullptr && commWorker->isRunning() )
    {
      return( true );
    }
    return( false );
  }

  QString BTDevice::getErrorText( int errnr, qint64 langId ) const
  {
    DWORD result;
    LPTSTR lpBuffer = nullptr;

    result = ::FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK,
      nullptr,
      static_cast<DWORD>(errnr),
      static_cast<DWORD>(langId),
      (LPTSTR)&lpBuffer,
      0,
      nullptr
    );

    if( result != 0 )
    {
      QString msg = QString::fromWCharArray( (wchar_t*)lpBuffer );
      LocalFree( (LPTSTR)lpBuffer );
      return( msg );
    }
    return( tr("unknown errorcode") );
  }

#if defined(SPX_DEBUGING)
  QByteArray& WinBTDevice::getRecBuffer(void)
  {
    return(recBuffer);
  }
#endif
  void BTDevice::killThreads(void)
  {
    removeDiscoverWorker();
    removeConnectWorker();
    removeCommWorker();
  }

  void BTDevice::removeDiscoverWorker(void)
  {
    //
    if( discoverWorker != nullptr )
    {
      disconnect( discoverWorker, 0, 0, 0 );
      if( discoverWorker->isRunning() )
      {
        discoverWorker->quit();
        if( discoverWorker->wait( 10000 ) )
        {
          delete (discoverWorker);
          discoverWorker = nullptr;
        }
        else
        {
          // PANIK, der Thread hört nicht auf
          lg->crit( "WinBTDevice::removeDiscoverThread -> discover thread can not canceled! ALERT!" );
          discoverWorker->deleteLater();
          discoverWorker = nullptr;
        }
      }
    }
    else
    {
      delete (discoverWorker);
      discoverWorker = nullptr;
    }
  }

  void BTDevice::removeConnectWorker(void)
  {
    //
    // Keine Nullpointer Exceptions riskieren
    //
    if( connectWorker != nullptr )
    {
      disconnect( connectWorker, 0, 0, 0 );
      if( connectWorker->isRunning() )
      {
        connectWorker->quit();
        if( connectWorker->wait( 10000 ) )
        {
          delete (connectWorker);
          connectWorker = nullptr;
        }
        else
        {
          // PANIK, der Thread hört nicht auf
          lg->crit( "WinBTDevice::removeConnectThread -> connection thread can not canceled! ALERT!" );
          connectWorker->deleteLater();
          connectWorker = nullptr;
        }
      }
      else
      {
        delete (connectWorker);
        connectWorker = nullptr;
      }
    }
  }

  void BTDevice::removeCommWorker(void)
  {
    //
    // Keine Nullpointer Exceptions riskieren
    //
    if( commWorker != nullptr )
    {
      disconnect( commWorker, 0, 0, 0 );
      if( commWorker->isRunning() )
      {
        commWorker->cancel();
        if( commWorker->wait( 10000 ) )
        {
          delete (commWorker);
          commWorker = nullptr;
        }
        else
        {
          // PANIK, der Thread hört nicht auf
          lg->crit( "WinBTDevice::removeCommWorker -> comm thread can not canceled! ALERT!" );
          commWorker->deleteLater();
          commWorker = nullptr;
        }
      }
      else
      {
        delete (commWorker);
        commWorker = nullptr;
      }
    }
  }

  void BTDevice::btDiscoveringSlot(void)
  {
    lg->debug( "WinBTDevice::btDiscoveringSlot...");
    emit btDiscoveringSig();
  }

  void BTDevice::btNewDeviceFoundSlot(BluetoothDeviceDescriber *device )
  {
    lg->debug(QString("WinBTDevice::btDeviceDiscoveredSlot -> device found, addr: <").append(device->getAddr()).append(">"));
    emit btDeviceDiscoveredSig(device);
  }

  void BTDevice::btDiscoverEndSlot(void)
  {
    lg->debug("WinBTDevice::btDiscoverEndSlot -> send signal, set delete thread");
    QTimer::singleShot(100, this, &BTDevice::removeConnectWorker);
    emit btDiscoverEndSig();
  }


  void BTDevice::connectingSlot(void)
  {
    lg->debug("WinBTDevice::connectSlot...");
    emit btConnectingSig();
  }

  void BTDevice::connectSlot(const QByteArray& dAddr)
  {
    lg->debug("WinBTDevice::connectSlot...");
    QTimer::singleShot(1, this, &BTDevice::startCommWorker);
    emit btConnectedSig(dAddr);
  }

  void BTDevice::disconnectSlot(void)
  {
    lg->debug("WinBTDevice::disconnectSlot...");
    //killThreads();
    emit btDisconnectSig();
  }

  void BTDevice::connectErrorSlot(int errnr)
  {
    lg->debug("WinBTDevice::connectErrorSlot...");
    emit btConnectErrorSig(errnr);
  }

  void BTDevice::btDataRecivedSlot( const char * data, int len )
  {
    lg->debug("WinBTDevice::btDataRecivedSlot...");
    recBuffer.append(data, len);
    emit btDataRecivedSig();
  }
}
