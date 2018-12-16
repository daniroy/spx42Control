#include "ConnectFragment.hpp"
#include "ui_ConnectFragment.h"

namespace spx
{
  SPXDeviceList ConnectFragment::spx42Devices{SPXDeviceList()};

  /**
   * @brief ConnectFragment::ConnectFragment
   * @param parent
   * @param logger
   * @param spx42Database
   * @param spxCfg
   * @param remSPX42
   */
  ConnectFragment::ConnectFragment( QWidget *parent,
                                    std::shared_ptr< Logger > logger,
                                    std::shared_ptr< SPX42Database > spx42Database,
                                    std::shared_ptr< SPX42Config > spxCfg,
                                    std::shared_ptr< SPX42RemotBtDevice > remSPX42 )
      : QWidget( parent )
      , IFragmentInterface( logger, spx42Database, spxCfg, remSPX42 )
      , ui( new Ui::connectForm )
      , errMsg( tr( "CONNECTION BLUETHOOTH ERROR: %1" ) )
      , discoverObj( std::unique_ptr< BtDiscoverRemoteDevice >( new BtDiscoverRemoteDevice( lg ) ) )
  {
    // merke mir ob das Programm das zum ersten mal macht...
    static bool isFirstStart{true};
    lg->debug( "ConnectFragment::ConnectFragment..." );
    //
    // Fragmenttitel Musterstring erzeugen
    //
    fragmentTitlePattern = tr( "CONNECTSTATE SPX42 Serial [%1] LIC: %2" );
    //
    // GUI machen
    //
    ui->setupUi( this );
    ui->connectProgressBar->setVisible( false );
    ui->editAliasesTableView->setVisible( false );
    //
    // geräte einlesen und combo liste füllen
    // hashliste mac, <name,alias>
    //
    spxDevicesAliasHash = database->getDeviceAliasHash();
    //
    onConfLicChangedSlot();
    //
    // GUI Signale
    //
    connect( spxConfig.get(), &SPX42Config::licenseChangedSig, this, &ConnectFragment::onConfLicChangedSlot );
    connect( ui->connectButton, &QPushButton::clicked, this, &ConnectFragment::onConnectButtonSlot );
    connect( ui->propertyPushButton, &QPushButton::clicked, this, &ConnectFragment::onPropertyButtonSlot );
    connect( ui->discoverPushButton, &QPushButton::clicked, this, &ConnectFragment::onDiscoverButtonSlot );
    connect( ui->deviceComboBox, static_cast< void ( QComboBox::* )( int ) >( &QComboBox::currentIndexChanged ), this,
             &ConnectFragment::onCurrentIndexChangedSlot );
    connect( discoverObj.get(), &BtDiscoverRemoteDevice::onDiscoveredDeviceSig, this, &ConnectFragment::onDiscoveredDeviceSlot );
    connect( discoverObj.get(), &BtDiscoverRemoteDevice::onDiscoverScanFinishedSig, this,
             &ConnectFragment::onDiscoverScanFinishedSlot );
    connect( remoteSPX42.get(), &SPX42RemotBtDevice::onStateChangedSig, this, &ConnectFragment::onOnlineStatusChangedSlot );
    connect( remoteSPX42.get(), &SPX42RemotBtDevice::onSocketErrorSig, this, &ConnectFragment::onSocketErrorSlot );
    connect( remoteSPX42.get(), &SPX42RemotBtDevice::onCommandRecivedSig, this, &ConnectFragment::onCommandRecivedSlot );
    //
    // setzte den Connectionsstatus
    //
    setGuiConnected( remoteSPX42->getConnectionStatus() == SPX42RemotBtDevice::SPX42_CONNECTED );
    // nach 200 ms discover starten, falls das das erste mal ist
    if ( isFirstStart )
    {
      QTimer::singleShot( 200, this, [=]() { this->onDiscoverButtonSlot(); } );
      isFirstStart = false;
    }
    else
    {
      fillDeviceCombo();
    }
  }

  /**
   * @brief ConnectFragment::~ConnectFragment
   */
  ConnectFragment::~ConnectFragment()
  {
    lg->debug( "ConnectFragment::~ConnectFragment..." );
    deactivateTab();
  }

  /**
   * @brief ConnectFragment::deactivateTab
   */
  void ConnectFragment::deactivateTab()
  {
    //
    // deaktiviere signale für reciver
    //
    disconnect( spxConfig.get(), nullptr, this, nullptr );
    disconnect( remoteSPX42.get(), nullptr, this, nullptr );
  }

  /**
   * @brief ConnectFragment::onOnlineStatusChangedSlot
   */
  void ConnectFragment::onOnlineStatusChangedSlot( bool )
  {
    setGuiConnected( ( remoteSPX42->getConnectionStatus() == SPX42RemotBtDevice::SPX42_CONNECTED ) ||
                     remoteSPX42->getConnectionStatus() == SPX42RemotBtDevice::SPX42_CONNECTING );
    //
    // gibt es den verbundenen Computer schon in der Datenbank
    //
    QString addr( remoteSPX42->getRemoteConnected() );
    if ( remoteSPX42->getConnectionStatus() == SPX42RemotBtDevice::SPX42_CONNECTED )
    {
      //
      // wenn der alias nicht in der Liste ist, in die Datenbank damit
      //
      if ( !spxDevicesAliasHash.contains( addr ) )
      {
        lg->debug( QString( "ConnectFragment::onOnlineStatusChangedSlot -> connected, add device %1 in database" ).arg( addr ) );
        auto descr = ConnectFragment::spx42Devices.value( addr );
        //
        // und rein damit
        //
        database->addAlias( descr.first, descr.second, descr.second, true );
        //
        // ein kleiner einzeiler zum update der Device-Box
        //
        QTimer::singleShot( 50, [this] {
          this->spxDevicesAliasHash = this->database->getDeviceAliasHash();
          fillDeviceCombo();
        } );
      }
      else
      {
        //
        // der ist schon in der Datenbank
        // setzte als letzte Verbindung
        //
        database->setLastConnected( addr );
      }
    }
    // TODO: evtl mehr machen
  }

  /**
   * @brief ConnectFragment::onSocketErrorSlot
   * @param btErr
   */
  void ConnectFragment::onSocketErrorSlot( QBluetoothSocket::SocketError btErr )
  {
    QString sendMsg;

    switch ( btErr )
    {
      case QBluetoothSocket::UnknownSocketError:
        sendMsg = errMsg.arg( tr( "UNKNOWN ERROR" ) );
        break;
      case QBluetoothSocket::NoSocketError:
        sendMsg = errMsg.arg( tr( "NO BT SOCKET FOUND" ) );
        break;
      case QBluetoothSocket::HostNotFoundError:
        sendMsg = errMsg.arg( tr( "REMOTE HOST NOT FOUND" ) );
        break;
      case QBluetoothSocket::RemoteHostClosedError:
        sendMsg = errMsg.arg( tr( "REMOTE HOST CLOSED CONNECTION" ) );
        break;
      default:
        sendMsg = errMsg.arg( tr( "COMMUNICATION ERROR" ) );
    }
    lg->debug( QString( "ConnectFragment::onSocketErrorSlot -> send error message <%1> to main window..." ).arg( sendMsg ) );
    emit onErrorgMessageSig( sendMsg, true );
  }

  /**
   * @brief ConnectFragment::onConfLicChangedSlot
   */
  void ConnectFragment::onConfLicChangedSlot()
  {
    setGuiConnected( remoteSPX42->getConnectionStatus() == SPX42RemotBtDevice::SPX42_CONNECTED );
  }

  /**
   * @brief ConnectFragment::onCloseDatabaseSlot
   */
  void ConnectFragment::onCloseDatabaseSlot()
  {
    // TODO: implementieren
  }

  /**
   * @brief ConnectFragment::onConnectButtonSlot
   */
  void ConnectFragment::onConnectButtonSlot()
  {
    bool isConnected = remoteSPX42->getConnectionStatus() == SPX42RemotBtDevice::SPX42_CONNECTED;
    //
    lg->debug( "ConnectFragment::onConnectButtonSlot -> connect/disconnect button clicked." );
    //
    // je nachdem ob das gerät verbunden oder getrennt ist
    //
    if ( isConnected )
    {
      lg->debug( "ConnectFragment::onConnectButtonSlot -> disconnect current connection" );
      remoteSPX42->endConnection();
    }
    else
    {
      //
      // stelle sicher, dass ein Eintrag ausgewählt ist
      //
      if ( ui->deviceComboBox->currentIndex() < 0 )
      {
        lg->warn( "ConnectFragment::onConnectButtonSlot -> not an device delected!" );
        emit onWarningMessageSig( tr( "NOT SELECTED A DEVICE TO CONNECT" ), true );
      }
      QString remoteMac = ui->deviceComboBox->itemData( ui->deviceComboBox->currentIndex() ).toString();
      lg->debug( QString( "ConnectFragment::onConnectButtonSlot -> connect to device <%1>" ).arg( remoteMac ) );
      //
      // blockiere mal die GUI solange
      // setGuiConnected( true );
      //
      // jetzt das remote objekt arbeiten lassen
      //
      // programm schmiert ab, ursache noch nicht geklärt
      // discoverObj->stopDiscover();
      remoteSPX42->endConnection();
      remoteSPX42->startConnection( remoteMac );
    }
  }

  /**
   * @brief ConnectFragment::onPropertyButtonSlot
   */
  void ConnectFragment::onPropertyButtonSlot()
  {
    lg->debug( "ConnectFragment::onPropertyButtonSlot -> property button clicked." );
    if ( remoteSPX42->getConnectionStatus() != SPX42RemotBtDevice::SPX42_CONNECTED )
    {
      ui->editAliasesTableView->setVisible( !ui->editAliasesTableView->isVisible() );
    }
  }

  /**
   * @brief ConnectFragment::onDiscoverButtonSlot
   */
  void ConnectFragment::onDiscoverButtonSlot()
  {
    lg->debug( "ConnectFragment::onDiscoverButtonSlot -> discover button clicked." );
    //
    // Liste löschen
    //
    ui->deviceComboBox->clear();
    ConnectFragment::spx42Devices.clear();
    ui->discoverPushButton->setEnabled( false );
    //
    // das Discovering starten
    //
    discoverObj->startDiscover();
  }

  /**
   * @brief ConnectFragment::onDiscoveredDeviceSlot
   * @param deviceInfo
   */
  void ConnectFragment::onDiscoveredDeviceSlot( const SPXDeviceDescr &deviceInfo )
  {
    lg->debug( "ConnectFragment::onDiscoveredDeviceSlot-> device found, try inserting in combo..." );
    //
    // Gerät zuerst in die Liste einfügen
    // dabei gehe ich davon aus das die discoverroutine auf doppelte Geräte schon
    // geprüft hat
    //
    addDeviceComboEntry( deviceInfo );
    trySetIndex();
  }

  /**
   * @brief ConnectFragment::onDiscoverScanFinishedSlot
   */
  void ConnectFragment::onDiscoverScanFinishedSlot()
  {
    lg->debug( "ConnectFragment::onDiscoverScanFinishedSlot-> discovering finished..." );
    ui->discoverPushButton->setEnabled( true );
    trySetIndex();
  }

  /**
   * @brief ConnectFragment::onCurrentIndexChangedSlot
   * @param index
   */
  void ConnectFragment::onCurrentIndexChangedSlot( int index )
  {
    lg->debug( QString( "ConnectFragment::onCurrentIndexChangedSlot -> index changed to <%1>. addr: <%2>" )
                   .arg( index, 2, 10, QChar( '0' ) )
                   .arg( ui->deviceComboBox->itemData( index ).toString() ) );
  }

  /**
   * @brief ConnectFragment::onCommandRecivedSlot
   */
  void ConnectFragment::onCommandRecivedSlot()
  {
    spSingleCommand recCommand;
    QDateTime nowDateTime;
    QByteArray value;
    char kdo;
    //
    lg->debug( "ConnectFragment::onDatagramRecivedSlot..." );
    //
    // alle abholen...
    //
    while ( ( recCommand = remoteSPX42->getNextRecCommand() ) )
    {
      // ja, es gab ein Datagram zum abholen
      kdo = recCommand->getCommand();
      switch ( kdo )
      {
        case SPX42CommandDef::SPX_ALIVE:
          // Kommando ALIVE liefert zurück:
          // ~03:PW
          // PX => Angabe HEX in Milivolt vom Akku
          lg->debug( "ConnectFragment::onDatagramRecivedSlot -> alive/acku..." );
          ackuVal = ( recCommand->getValueAt( SPXCmdParam::ALIVE_POWER ) / 100.0 );
          emit onAkkuValueChangedSig( ackuVal );
          break;
        case SPX42CommandDef::SPX_APPLICATION_ID:
          // Kommando APPLICATION_ID (VERSION)
          // ~04:NR -> VERSION als String
          lg->debug( "ConnectFragment::onDatagramRecivedSlot -> firmwareversion..." );
          // Setzte die Version in die Config
          spxConfig->setSpxFirmwareVersion( recCommand->getParamAt( SPXCmdParam::FIRMWARE_VERSION ) );
          spxConfig->freezeConfigs( SPX42ConfigClass::CF_CLASS_SPX );
          // Geht das Datum zu setzen?
          if ( spxConfig->getCanSetDate() )
          {
            // ja der kann das Datum online setzten
            nowDateTime = QDateTime::currentDateTime();
            // sende das Datum an den SPX
            remoteSPX42->setDateTime( nowDateTime );
          }
          break;
        case SPX42CommandDef::SPX_SERIAL_NUMBER:
          // Kommando SERIAL NUMBER
          // ~07:XXX -> Seriennummer als String
          lg->debug( "ConnectFragment::onDatagramRecivedSlot -> serialnumber..." );
          spxConfig->setSerialNumber( recCommand->getParamAt( SPXCmdParam::SERIAL_NUMBER ) );
          spxConfig->freezeConfigs( SPX42ConfigClass::CF_CLASS_SPX );
          setGuiConnected( remoteSPX42->getConnectionStatus() == SPX42RemotBtDevice::SPX42_CONNECTED );
          break;
        case SPX42CommandDef::SPX_LICENSE_STATE:
          // Kommando SPX_LICENSE_STATE
          // komplett: <~45:LS:CE>
          // übergeben LS,CE
          // LS : License State 0=Nitrox,1=Normoxic Trimix,2=Full Trimix
          // CE : Custom Enabled 0= disabled, 1=enabled
          lg->debug( "ConnectFragment::onDatagramRecivedSlot -> license state..." );
          spxConfig->setLicense( recCommand->getParamAt( SPXCmdParam::LICENSE_STATE ),
                                 recCommand->getParamAt( SPXCmdParam::LICENSE_INDIVIDUAL ) );
          spxConfig->freezeConfigs( SPX42ConfigClass::CF_CLASS_SPX );
          setGuiConnected( remoteSPX42->getConnectionStatus() == SPX42RemotBtDevice::SPX42_CONNECTED );
          break;
      }
      //
      // falls es mehr gibt, lass dem Rest der App auch eine Chance
      //
      QCoreApplication::processEvents();
    }
  }

  // ##########################################################################
  // PRIVATE Funktionen
  // ##########################################################################

  /**
   * @brief ConnectFragment::setGuiConnected
   * @param isConnected
   */
  void ConnectFragment::setGuiConnected( bool isConnected )
  {
    //
    // setzte die GUI Elemente entsprechend des Online Status
    //
    isConnected ? ui->connectButton->setText( tr( "DISCONNECT DEVICE" ) ) : ui->connectButton->setText( tr( "CONNECT DEVICE" ) );
    ui->deviceComboBox->setEnabled( !isConnected );
    ui->propertyPushButton->setEnabled( !isConnected );
    ui->discoverPushButton->setEnabled( !isConnected );
    if ( isConnected )
      ui->editAliasesTableView->setVisible( false );
    ui->tabHeaderLabel->setText( fragmentTitlePattern.arg( spxConfig->getSerialNumber() ).arg( spxConfig->getLicName() ) );
  }

  /**
   * @brief ConnectFragment::addDeviceComboEntry
   * @param deviceInfo
   */
  void ConnectFragment::addDeviceComboEntry( const SPXDeviceDescr &deviceInfo )
  {
    QString addr = deviceInfo.first;
    ConnectFragment::spx42Devices.insert( addr, deviceInfo );
    //
    // und nun eine vernünftige Anzeige machen und die MAC als Schlüssel lassen
    //
    QString title = addr;  // TODO: aus den Aliasen was bauen

    if ( spxDevicesAliasHash.contains( addr ) )
    {
      //
      // ein alias ist vorhanden!
      //
      auto localSpxDevice = spxDevicesAliasHash.value( addr );
      title = QString( "%1 (%2)" ).arg( localSpxDevice.alias ).arg( localSpxDevice.name );
    }
    else
    {
      //
      // es gibt (noch) keinen Alias, baue den Eintrag aus Name und MAC
      //
      title = QString( "%1 (%2)" ).arg( deviceInfo.second ).arg( addr );
    }
    ui->deviceComboBox->addItem( title, addr );
  }

  void ConnectFragment::fillDeviceCombo()
  {
    //
    ui->deviceComboBox->clear();
    for ( auto deviceInfo : ConnectFragment::spx42Devices )
    {
      addDeviceComboEntry( deviceInfo );
    }
    trySetIndex();
  }

  void ConnectFragment::trySetIndex()
  {
    //
    // wer ist verbunden
    //
    QString mac;
    lg->debug( "ConnectFragment::trySetIndex..." );
    mac = remoteSPX42->getRemoteConnected();
    if ( mac.isEmpty() )
    {
      //
      // nichts verbunden
      // gucke ob ich das zulettz verbundene Gerät finde
      //
      lg->debug( "ConnectFragment::trySetIndex -> not connected, try last connected..." );
      mac = database->getLastConnected();
    }
    if ( !mac.isEmpty() )
    {
      lg->debug( QString( "ConnectFragment::trySetIndex -> last connected was: " ).append( mac ) );
      //
      // verbunden oder nicht, versuche etwas zu selektiern
      //
      // suche nach diesem Eintrag...
      //
      int index = ui->deviceComboBox->findData( mac );
      if ( index != ui->deviceComboBox->currentIndex() && index != -1 )
      {
        lg->debug( QString( "ConnectFragment::trySetIndex -> found at idx %1, set to idx" ).arg( index ) );
        //
        // -1 for not found
        // und der index ist nicht schon auf diesen Wert gesetzt
        // also, wenn gefunden, selektiere diesen Eintrag
        //
        ui->deviceComboBox->setCurrentIndex( index );
      }
    }
    lg->debug( "ConnectFragment::trySetIndex...OK" );
  }
}  // namespace spx
