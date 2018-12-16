#include "GasFragment.hpp"
#include "ui_GasFragment.h"

namespace spx
{
  GasFragment::GasFragment( QWidget *parent,
                            std::shared_ptr< Logger > logger,
                            std::shared_ptr< SPX42Database > spx42Database,
                            std::shared_ptr< SPX42Config > spxCfg,
                            std::shared_ptr< SPX42RemotBtDevice > remSPX42 )
      : QWidget( parent )
      , IFragmentInterface( logger, spx42Database, spxCfg, remSPX42 )
      , ui( new Ui::GasForm )
      , areSlotsConnected( false )
      , currentGasNum( 0 )
      , ignoreGasGuiEvents( false )
      , waterCompute( DeviceWaterType::FRESHWATER )
  {
    lg->debug( "GasFragment::GasFragment..." );
    gasSummaryTemplate = tr( "GAS #%1 HE:%2 O2:%3 N2:%4 DIL1 [%5] DIL2 [%6] BO [%7]" );
    gasCurrentBoxTitleTemplate = tr( "GAS #%1" );
    ui->setupUi( this );
    ui->transferProgressBar->setVisible( false );
    initGuiWithConfig();
    onConfLicChangedSlot();
    connectSlots();
    connect( spxConfig.get(), &SPX42Config::licenseChangedSig, this, &GasFragment::onConfLicChangedSlot );
    setGuiConnected( remoteSPX42->getConnectionStatus() == SPX42RemotBtDevice::SPX42_CONNECTED );
    if ( remoteSPX42->getConnectionStatus() == SPX42RemotBtDevice::SPX42_CONNECTED )
      QTimer::singleShot( 250, this, &GasFragment::onGasConfigUpdateSlot );
  }

  GasFragment::~GasFragment()
  {
    lg->debug( "GasFragment::~GasFragment..." );
    deactivateTab();
    // delete ui;
  }

  void GasFragment::deactivateTab()
  {
    disconnect( spxConfig.get(), nullptr, this, nullptr );
  }

  void GasFragment::initGuiWithConfig()
  {
    bool wasSlotsConnected = areSlotsConnected;
    //
    // Gase sind im Config Objekt enthalten, also erst mal die GUI nach den Gasen aufbauen
    // Das sollte VOR dem Verbinden der Slots passieren
    // ODER disconnectSlots() => initGuiWithConfig() => connectSlots()
    //
    if ( wasSlotsConnected )
    {
      disconnectSlots();
    }
    //
    // gas listen
    //
    gasRadios.insert( 0, ui->gas01RadioButton );
    gasRadios.insert( 1, ui->gas02RadioButton );
    gasRadios.insert( 2, ui->gas03RadioButton );
    gasRadios.insert( 3, ui->gas04RadioButton );
    gasRadios.insert( 4, ui->gas05RadioButton );
    gasRadios.insert( 5, ui->gas06RadioButton );
    gasRadios.insert( 6, ui->gas07RadioButton );
    gasRadios.insert( 7, ui->gas08RadioButton );
    //
    // Voreinstellung
    //
    ui->currWaterTypeCheckBox->setChecked( true );
    ui->currWaterTypeCheckBox->setText( tr( "COMPUTE FRESHWATER" ) );
    //
    if ( wasSlotsConnected )
    {
      connectSlots();
    }
    //
    // Onlinestsatus
    //
    setGuiConnected( remoteSPX42->getConnectionStatus() == SPX42RemotBtDevice::SPX42_CONNECTED );
  }

  void GasFragment::setGuiConnected( bool connected )
  {
    disconnectSlots();
    ui->tabHeaderLabel->setText(
        QString( tr( "GASLIST SPX42 SERIAL [%1] LIC: %2" ).arg( spxConfig->getSerialNumber() ).arg( spxConfig->getLicName() ) ) );
    if ( connected )
      currentGasNum = 0;
    else
      spxConfig->setLicense( LicenseType::LIC_FULLTMX );
    //
    // GUI ist ONLINE/OFFLINE
    //
    for ( auto i = 0; i < 8; i++ )
    {
      gasRadios.at( i )->setVisible( connected );
    }
    gasRadios.at( currentGasNum )->setChecked( true );
    if ( connected )
    {
      //
      // ist nur online zu sehen, also auch nicht zu updaten
      //
      for ( auto i = 0; i < 8; i++ )
      {
        if ( currentGasNum == i )
          continue;
        else
          updateCurrGasGUI( i, false );
      }
    }
    updateCurrGasGUI( currentGasNum );
    ui->currdil01CheckBox->setEnabled( connected );
    ui->currdil02CheckBox->setEnabled( connected );
    ui->currbailoutCheckBox->setEnabled( connected );
    connectSlots();
  }

  void GasFragment::connectSlots()
  {
    //
    // die Gase zum auswählen via radio box
    //
    connect( ui->gas01RadioButton, &QRadioButton::toggled, [=]( bool isSelected ) { this->gasSelect( 0, isSelected ); } );
    connect( ui->gas02RadioButton, &QRadioButton::toggled, [=]( bool isSelected ) { this->gasSelect( 1, isSelected ); } );
    connect( ui->gas03RadioButton, &QRadioButton::toggled, [=]( bool isSelected ) { this->gasSelect( 2, isSelected ); } );
    connect( ui->gas04RadioButton, &QRadioButton::toggled, [=]( bool isSelected ) { this->gasSelect( 3, isSelected ); } );
    connect( ui->gas05RadioButton, &QRadioButton::toggled, [=]( bool isSelected ) { this->gasSelect( 4, isSelected ); } );
    connect( ui->gas06RadioButton, &QRadioButton::toggled, [=]( bool isSelected ) { this->gasSelect( 5, isSelected ); } );
    connect( ui->gas07RadioButton, &QRadioButton::toggled, [=]( bool isSelected ) { this->gasSelect( 6, isSelected ); } );
    connect( ui->gas08RadioButton, &QRadioButton::toggled, [=]( bool isSelected ) { this->gasSelect( 7, isSelected ); } );
    // Lizenz
    connect( spxConfig.get(), &SPX42Config::licenseChangedSig, this, &GasFragment::onConfLicChangedSlot );
    // CONFIG
    connect( remoteSPX42.get(), &SPX42RemotBtDevice::onStateChangedSig, this, &GasFragment::onOnlineStatusChangedSlot );
    connect( remoteSPX42.get(), &SPX42RemotBtDevice::onSocketErrorSig, this, &GasFragment::onSocketErrorSlot );
    connect( remoteSPX42.get(), &SPX42RemotBtDevice::onCommandRecivedSig, this, &GasFragment::onCommandRecivedSlot );
    //
    // die gas spinbioxen für das aktuelle gas
    //
    connect( ui->currO2spinBox, QSpinboxIntValueChanged, this, &GasFragment::onSpinO2ValueChangedSlot );
    connect( ui->currHESpinBox, QSpinboxIntValueChanged, this, &GasFragment::onSpinHeValueChangedSlot );
    connect( ui->currWaterTypeCheckBox, &QCheckBox::stateChanged, this, &GasFragment::onWaterTypeChanged );
    connect( ui->currbailoutCheckBox, &QCheckBox::stateChanged, this, &GasFragment::onBailoutCheckChangeSlot );
    connect( ui->currdil01CheckBox, &QCheckBox::stateChanged,
             [=]( int state ) { this->onDiluentUseChangeSlot( state, DiluentType::DIL_01 ); } );
    connect( ui->currdil02CheckBox, &QCheckBox::stateChanged,
             [=]( int state ) { this->onDiluentUseChangeSlot( state, DiluentType::DIL_02 ); } );
  }

  void GasFragment::disconnectSlots()
  {
    //
    // Alle radios trennen
    //
    disconnect( ui->gas01RadioButton, nullptr, nullptr, nullptr );
    disconnect( ui->gas02RadioButton, nullptr, nullptr, nullptr );
    disconnect( ui->gas03RadioButton, nullptr, nullptr, nullptr );
    disconnect( ui->gas04RadioButton, nullptr, nullptr, nullptr );
    disconnect( ui->gas05RadioButton, nullptr, nullptr, nullptr );
    disconnect( ui->gas06RadioButton, nullptr, nullptr, nullptr );
    disconnect( ui->gas07RadioButton, nullptr, nullptr, nullptr );
    disconnect( ui->gas08RadioButton, nullptr, nullptr, nullptr );
    // Lizenz
    disconnect( spxConfig.get(), nullptr, this, nullptr );
    // CONFIG
    disconnect( remoteSPX42.get(), nullptr, this, nullptr );
    //
    // die gas spinbioxen für das aktuelle gas
    //
    disconnect( ui->currO2spinBox, nullptr, nullptr, nullptr );
    disconnect( ui->currHESpinBox, nullptr, nullptr, nullptr );
    disconnect( ui->currWaterTypeCheckBox, nullptr, nullptr, nullptr );
    disconnect( ui->currbailoutCheckBox, nullptr, nullptr, nullptr );
    disconnect( ui->currdil01CheckBox, nullptr, nullptr, nullptr );
    disconnect( ui->currdil02CheckBox, nullptr, nullptr, nullptr );
  }

  void GasFragment::onSpinO2ValueChangedSlot( int o2Val )
  {
    lg->debug( QString( "GasFragment::onSpinO2ValueChangedSlot -> gas nr <%1> was changed..." )
                   .arg( currentGasNum + 1, 2, 10, QChar( '0' ) ) );
    //
    // Gas setzen, Plausibilität prüfen, ggf korrigieren
    //
    SPX42Gas currGas = spxConfig->getGasAt( currentGasNum );
    currGas.setO2( static_cast< quint8 >( o2Val ), spxConfig->getLicense().getLicType() );
    spxConfig->setGasAt( currentGasNum, currGas );
    if ( remoteSPX42->getConnectionStatus() == SPX42RemotBtDevice::SPX42_CONNECTED )
      emit onConfigWasChangedSig();
    disconnectSlots();
    updateCurrGasGUI( currentGasNum );
    connectSlots();
  }

  void GasFragment::onSpinHeValueChangedSlot( int heVal )
  {
    lg->debug( QString( "GasFragment::onSpinHeValueChangedSlot -> gas nr <%1> was changed..." )
                   .arg( currentGasNum + 1, 2, 10, QChar( '0' ) ) );
    //
    // Gas setzen, Plausibilität prüfen, ggf korrigieren
    //
    SPX42Gas currGas = spxConfig->getGasAt( currentGasNum );
    currGas.setHe( static_cast< quint8 >( heVal ), spxConfig->getLicense().getLicType() );
    spxConfig->setGasAt( currentGasNum, currGas );
    if ( remoteSPX42->getConnectionStatus() == SPX42RemotBtDevice::SPX42_CONNECTED )
      emit onConfigWasChangedSig();
    disconnectSlots();
    updateCurrGasGUI( currentGasNum );
    connectSlots();
  }

  void GasFragment::updateCurrGasGUI( int gasNum, bool withCurrent )
  {
    //
    // Plausibilität checken, GUI korrigieren
    //
    SPX42Gas spxGas = spxConfig->getGasAt( gasNum );
    int currO2 = spxGas.getO2();
    int currHe = spxGas.getHe();
    int currN2 = spxGas.getN2();
    //"GAS #%1 HE:%2 O2:%3 N2:%4 DIL1 [%5] DIL2 [%6] BO [%7]"
    gasRadios.at( gasNum )->setText( gasSummaryTemplate.arg( gasNum + 1, 2, 10, QChar( '0' ) )
                                         .arg( currHe, 2, 10, QChar( '0' ) )
                                         .arg( currO2, 2, 10, QChar( '0' ) )
                                         .arg( currN2, 2, 10, QChar( '0' ) )
                                         .arg( spxGas.getDiluentType() == DiluentType::DIL_01 ? "X" : " " )
                                         .arg( spxGas.getDiluentType() == DiluentType::DIL_02 ? "X" : " " )
                                         .arg( spxGas.getBailout() ? "X" : " " ) );
    //
    // da hier nur wenn der aktuelle teil auch aktualisiert werden soll
    // (ist default)
    //
    if ( withCurrent )
    {
      ui->currGasGroupBox->setTitle( gasCurrentBoxTitleTemplate.arg( gasNum + 1, 2, 10, QChar( '0' ) ) );
      ui->currN2LineEdit->setText( QString( "%1" ).arg( currN2, 2, 10, QChar( '0' ) ) );
      ui->currNameLabel->setText( spxGas.getGasName() );
      if ( ui->currO2spinBox->value() != currO2 )
        ui->currO2spinBox->setValue( currO2 );
      if ( ui->currHESpinBox->value() != currHe )
        ui->currHESpinBox->setValue( currHe );
      qint16 mod = spxGas.getMOD( waterCompute );
      ui->currentMODLabel->setText( tr( "MOD: %1m" ).arg( mod, 2, 10, QChar( '0' ) ) );
      ui->currentEADLabel->setText( tr( "EAD: %1m" ).arg( spxGas.getEAD( mod, waterCompute ), 2, 10, QChar( '0' ) ) );
      ui->currdil01CheckBox->setChecked( spxGas.getDiluentType() == DiluentType::DIL_01 ? true : false );
      ui->currdil02CheckBox->setChecked( spxGas.getDiluentType() == DiluentType::DIL_02 ? true : false );
      ui->currbailoutCheckBox->setChecked( spxGas.getBailout() );
    }
  }

  void GasFragment::onDiluentUseChangeSlot( int state, DiluentType which )
  {
    //
    // Alle anderen Gase dürfen dann NICHT which sein!
    // Nur ein DIL1, beliegig DIL2 aber nicht DIL1 == DIL2
    //
    lg->debug( QString( "GasFragment::onDiluentUseChangeSlot -> gas nr <%1> , DIL <%2> was changed to <%3>..." )
                   .arg( currentGasNum + 1, 2, 10, QChar( '0' ) )
                   .arg( static_cast< int >( which ), 2, 10, QChar( '0' ) )
                   .arg( state, 2, 10, QChar( '0' ) ) );
    //
    // setzten oder löschen des Gases?
    //
    if ( state == Qt::Checked )
    {
      //
      // Gas setzten
      // welches GAS
      //
      if ( which == DiluentType::DIL_01 )
      {
        // alle DIL_01 löschen, nur meins bleibt aktiv
        for ( auto i = 0; i < 8; i++ )
        {
          if ( spxConfig->getGasAt( i ).getDiluentType() == DiluentType::DIL_01 )
          {
            SPX42Gas chGas( spxConfig->getGasAt( i ) );
            chGas.setDiluentType( DiluentType::DIL_NONE );
            spxConfig->setGasAt( i, chGas );
          }
        }
        SPX42Gas chGas( spxConfig->getGasAt( currentGasNum ) );
        chGas.setDiluentType( DiluentType::DIL_01 );
        spxConfig->setGasAt( currentGasNum, chGas );
      }
      else if ( which == DiluentType::DIL_02 )
      {
        SPX42Gas chGas( spxConfig->getGasAt( currentGasNum ) );
        chGas.setDiluentType( DiluentType::DIL_02 );
        spxConfig->setGasAt( currentGasNum, chGas );
      }
      else
      {
        //
        // eigentlich obsolte...
        //
        SPX42Gas chGas( spxConfig->getGasAt( currentGasNum ) );
        chGas.setDiluentType( DiluentType::DIL_NONE );
        spxConfig->setGasAt( currentGasNum, chGas );
      }
    }
    else
    {
      //
      // unset DILUENT
      //
      SPX42Gas chGas( spxConfig->getGasAt( currentGasNum ) );
      chGas.setDiluentType( DiluentType::DIL_NONE );
      spxConfig->setGasAt( currentGasNum, chGas );
    }
    //
    // jetzt noch gewährleisten, dass mindestens ein DIL1 da ist
    //
    bool setGas01ToDIL1 = true;
    for ( auto i = 0; i < 8; i++ )
    {
      if ( spxConfig->getGasAt( i ).getDiluentType() == DiluentType::DIL_01 )
      {
        setGas01ToDIL1 = false;
        break;
      }
    }
    //
    // wenn nun kein DIL1 mehr vorhanden war, setzte GAS01 als DIL1
    //
    if ( setGas01ToDIL1 )
    {
      SPX42Gas chGas( spxConfig->getGasAt( 0 ) );
      chGas.setDiluentType( DiluentType::DIL_NONE );
      spxConfig->setGasAt( 0, chGas );
    }
    //
    // GUI update
    //
    disconnectSlots();
    for ( auto i = 0; i < 8; i++ )
    {
      if ( currentGasNum == i )
        continue;
      else
        updateCurrGasGUI( i, false );
    }
    if ( remoteSPX42->getConnectionStatus() == SPX42RemotBtDevice::SPX42_CONNECTED )
      emit onConfigWasChangedSig();
    updateCurrGasGUI( currentGasNum );
    connectSlots();
  }

  void GasFragment::onBailoutCheckChangeSlot( int state )
  {
    SPX42Gas chGas = spxConfig->getGasAt( currentGasNum );
    lg->debug(
        QString( "GasFragment::onBaCheckChangeSlot -> gas nr <%1> was changed..." ).arg( currentGasNum + 1, 2, 10, QChar( '0' ) ) );
    if ( state == Qt::Checked )
    {
      chGas.setBailout( true );
    }
    else
    {
      chGas.setBailout( true );
    }
    spxConfig->setGasAt( currentGasNum, chGas );
    if ( remoteSPX42->getConnectionStatus() == SPX42RemotBtDevice::SPX42_CONNECTED )
      emit onConfigWasChangedSig();
    disconnectSlots();
    updateCurrGasGUI( currentGasNum );
    connectSlots();
  }

  void GasFragment::onConfLicChangedSlot()
  {
    lg->debug(
        QString( "GasFragment::onConfLicChangedSlot -> set: %1" ).arg( static_cast< int >( spxConfig->getLicense().getLicType() ) ) );
    ui->tabHeaderLabel->setText(
        QString( tr( "GASLIST SPX42 Serial [%1] Lic: %2" ).arg( spxConfig->getSerialNumber() ).arg( spxConfig->getLicName() ) ) );
  }

  void GasFragment::onOnlineStatusChangedSlot( bool )
  {
    setGuiConnected( remoteSPX42->getConnectionStatus() == SPX42RemotBtDevice::SPX42_CONNECTED );
  }

  void GasFragment::onSocketErrorSlot( QBluetoothSocket::SocketError )
  {
    // TODO: implementieren
  }

  void GasFragment::onCloseDatabaseSlot()
  {
    // TODO: implementieren
  }

  void GasFragment::onCommandRecivedSlot()
  {
    spSingleCommand recCommand;
    QDateTime nowDateTime;
    QByteArray value;
    SPX42Gas recGas;
    int recGasNumber, n2, he, o2;

    char kdo;
    //
    lg->debug( "GasFragment::onCommandRecivedSlot..." );
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
          lg->debug( "GasFragment::onCommandRecivedSlot -> alive/acku..." );
          ackuVal = ( recCommand->getValueAt( SPXCmdParam::ALIVE_POWER ) / 100.0 );
          emit onAkkuValueChangedSig( ackuVal );
          break;
        case SPX42CommandDef::SPX_APPLICATION_ID:
          // Kommando APPLICATION_ID (VERSION)
          // ~04:NR -> VERSION als String
          lg->debug( "GasFragment::onCommandRecivedSlot -> firmwareversion..." );
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
          lg->debug( "GasFragment::onCommandRecivedSlot -> serialnumber..." );
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
          lg->debug( "GasFragment::onCommandRecivedSlot -> license state..." );
          spxConfig->setLicense( recCommand->getParamAt( SPXCmdParam::LICENSE_STATE ),
                                 recCommand->getParamAt( SPXCmdParam::LICENSE_INDIVIDUAL ) );
          spxConfig->freezeConfigs( SPX42ConfigClass::CF_CLASS_SPX );
          setGuiConnected( remoteSPX42->getConnectionStatus() == SPX42RemotBtDevice::SPX42_CONNECTED );
          break;
        case SPX42CommandDef::SPX_GET_SETUP_GASLIST:
          // Kommando GET_SETUP_GASLIST
          // <~39:NR:ST:HE:BA:AA:CG>
          // NR: Numer des Gases
          // ST Stickstoff in Prozent (hex)
          // HELIUM
          // Bailout
          // AA Diluent 1 oder 2 oder keins
          // CG curent Gas
          recGasNumber = static_cast< int >( recCommand->getValueAt( SPXCmdParam::GASLIST_GAS_NUMBER ) );
          n2 = static_cast< int >( recCommand->getValueAt( SPXCmdParam::GASLIST_N2 ) );
          he = static_cast< int >( recCommand->getValueAt( SPXCmdParam::GASLIST_HE ) );
          o2 = 100 - ( n2 + he );
          lg->debug( QString( "GasFragment::onCommandRecivedSlot -> gas #%1..." ).arg( recGasNumber, 2, 10, QChar( '0' ) ) );
          // CONFIG speichern
          disconnectSlots();
          recGas.reset();
          recGas.setO2( static_cast< quint8 >( o2 ) );
          recGas.setHe( static_cast< quint8 >( he ) );
          recGas.setBailout( recCommand->getValueAt( SPXCmdParam::GASLIST_BAILOUT ) == 0 ? false : true );
          recGas.setDiluentType( static_cast< DiluentType >( recCommand->getValueAt( SPXCmdParam::GASLIST_DILUENT ) ) );
          spxConfig->setGasAt( recGasNumber, recGas );
          if ( recCommand->getValueAt( SPXCmdParam::GASLIST_IS_CURRENT ) > 0 )
          {
            // current parameter ist hier immer 0
          }
          spxConfig->freezeConfigs( SPX42ConfigClass::CF_CLASS_GASES );
          // GUI update
          updateCurrGasGUI( recGasNumber );
          connectSlots();
          break;
      }
    }
  }

  void GasFragment::onGasConfigUpdateSlot()
  {
    //
    // rufe alle gase vom computer ab
    //
    SendListEntry sendCommand = remoteSPX42->askForGasList();
    remoteSPX42->sendCommand( sendCommand );
    lg->debug( "GasFragment::onGasConfigUpdateSlot -> send cmd all gases..." );
  }

  void GasFragment::gasSelect( int gasNum, bool isSelected )
  {
    if ( isSelected )
    {
      //
      // welche Nummer bearbeiten?
      //
      currentGasNum = gasNum;
      // sag bescheid
      lg->debug( QString( "GasFragment::gasSelect -> gas #%1" ).arg( gasNum + 1, 2, 10, QChar( '0' ) ) );
    }
    //
    //"GAS #%1 HE:%2 O2:%3 N2:%4 DIL1 [%5] DIL2 [%6] BO [%7]"
    // für micht anzeigen
    disconnectSlots();
    updateCurrGasGUI( currentGasNum );
    connectSlots();
  }

  void GasFragment::onWaterTypeChanged( int state )
  {
    if ( state )
    {
      ui->currWaterTypeCheckBox->setText( tr( "COMPUTE FRESHWATER" ) );
      waterCompute = DeviceWaterType::FRESHWATER;
    }
    else
    {
      ui->currWaterTypeCheckBox->setText( tr( "COMPUTE SALTWATER" ) );
      waterCompute = DeviceWaterType::SALTWATER;
    }
    disconnectSlots();
    updateCurrGasGUI( currentGasNum );
    connectSlots();
  }

}  // namespace spx
