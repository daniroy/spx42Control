#include "DeviceConfigFragment.hpp"
#include "ui_DeviceConfigFragment.h"

namespace spx
{
  /**
   * @brief Konstruktor für Konfigurationsfragment
   * @param parent Parentobjekt
   * @param logger Logger
   * @param spxCfg SPX42 Konfigurationsobjekt
   */
  DeviceConfigFragment::DeviceConfigFragment( QWidget *parent,
                                              std::shared_ptr< Logger > logger,
                                              std::shared_ptr< SPX42Database > spx42Database,
                                              std::shared_ptr< SPX42Config > spxCfg,
                                              std::shared_ptr< SPX42RemotBtDevice > remSPX42 )
      : QWidget( parent )
      , IFragmentInterface( logger, spx42Database, spxCfg, remSPX42 )
      , ui( new Ui::DeviceConfig )
      , gradentSlotsIgnore( false )
      , oldAutoSetpoint()
      , newAutoSetpoint()
  {
    configHeadlineTemplate = tr( "CONFIG SPX42 SERIAL [%1] LIC: %2" );
    lg->debug( QString( "DeviceConfigFragment::DeviceConfigFragment -> device connected: %1..." )
                   .arg( remoteSPX42->getConnectionStatus() == SPX42RemotBtDevice::SPX42_CONNECTED ) );
    ui->setupUi( this );
    ui->transferProgressBar->setVisible( false );
    oldAutoSetpoint << tr( "OFF" ) << tr( "5 METERS" ) << tr( "10 METERS" ) << tr( "15 METERS" ) << tr( "20 METERS" );
    newAutoSetpoint << tr( "OFF" ) << tr( "6 METERS" ) << tr( "10 METERS" ) << tr( "15 METERS" ) << tr( "20 METERS" );
    oldDisplayBrightness << tr( "10%" ) << tr( "50%" ) << tr( "100%" );
    newDisplayBrightness << tr( "20%" ) << tr( "40%" ) << tr( "60%" ) << tr( "80%" ) << tr( "100%" );
    initGuiWithConfig();
    setGuiConnected( remoteSPX42->getConnectionStatus() == SPX42RemotBtDevice::SPX42_CONNECTED );
    updateGuiFirmwareSpecific();
    connectSlots();  // alle slots aktivieren
    if ( remoteSPX42->getConnectionStatus() == SPX42RemotBtDevice::SPX42_CONNECTED )
      QTimer::singleShot( 250, this, &DeviceConfigFragment::onConfigUpdateSlot );
  }

  /**
   * @brief Destruktor, aufräumen
   */
  DeviceConfigFragment::~DeviceConfigFragment()
  {
    lg->debug( "DeviceConfigFragment::~DeviceConfigFragment..." );
    deactivateTab();
  }

  void DeviceConfigFragment::deactivateTab( void )
  {
    // alle signale trennen...
    disconnectSlots();
  }

  /**
   * @brief Initialisiere das Fragment (GUI Aufbauen)
   */
  void DeviceConfigFragment::initGuiWithConfig()
  {
    //
    // Initialisiere die GUI
    //
    ui->tabHeaderLabel->setText(
        QString( configHeadlineTemplate.arg( spxConfig->getSerialNumber() ).arg( spxConfig->getLicName() ) ) );
    //
    // Dekompression
    //
    setGuiForDecompression();
    //
    // Display von config
    //
    setGuiForDisplay();
    //
    // Einheiten von config
    //
    setGuiForUnits();
    //
    setGuiForSetpoint();
    //
    setGuiForLicense();  // individual wird hier implizit mit aufgerufen!
  }

  /**
   * @brief GUI für Dekomprtession nach spxConfig einrichten
   */
  void DeviceConfigFragment::setGuiForDecompression()
  {
    //
    // Signale vorher trennen um Schleifen zu vermeiden
    //
    disconnectSlots( SIGNALS_DECOMPRESSION );
    //
    DecompressionPreset cPreset = spxConfig->getCurrentDecoGradientPresetType();
    DecoGradient newGrad = spxConfig->getPresetValues( spxConfig->getCurrentDecoGradientPresetType() );
    ui->conservatismComboBox->setCurrentIndex( static_cast< int >( cPreset ) );
    ui->gradientLowSpinBox->setValue( newGrad.first );
    ui->gradientHighSpinBox->setValue( newGrad.second );
    if ( DecompressionDeepstops::DEEPSTOPS_ENABLED == spxConfig->getIstDeepstopsEnabled() )
    {
      ui->deepStopOnCheckBox->setCheckState( Qt::CheckState::Checked );
    }
    else
    {
      ui->deepStopOnCheckBox->setCheckState( Qt::CheckState::Unchecked );
    }
    if ( DecompressionDynamicGradient::DYNAMIC_GRADIENT_ON == spxConfig->getIsDecoDynamicGradients() )
    {
      ui->dynamicGradientsOnCheckBox->setCheckState( Qt::CheckState::Checked );
    }
    else
    {
      ui->dynamicGradientsOnCheckBox->setCheckState( Qt::CheckState::Unchecked );
    }
    // Signale mit Slots verbinden
    connectSlots( SIGNALS_DECOMPRESSION );
  }

  /**
   * @brief GUI für Display nach spxConfig einrichten
   */
  void DeviceConfigFragment::setGuiForDisplay()
  {
    disconnectSlots( SIGNALS_DISPLAY );
    //
    ui->displayBrightnessComboBox->setCurrentIndex( static_cast< int >( spxConfig->getDisplayBrightness() ) );
    ui->displayOrientationComboBox->setCurrentIndex( static_cast< int >( spxConfig->getDisplayOrientation() ) );
    //
    connectSlots( SIGNALS_DISPLAY );
  }

  /**
   * @brief GUI für Einheiten nach spxConfig einrichten
   */
  void DeviceConfigFragment::setGuiForUnits()
  {
    disconnectSlots( SIGNALS_UNITS );
    //
    ui->unitsTemperaturComboBox->setCurrentIndex( static_cast< int >( spxConfig->getUnitsTemperatur() ) );
    ui->unitsDeepComboBox->setCurrentIndex( static_cast< int >( spxConfig->getUnitsLength() ) );
    ui->unitsWaterTypeComboBox->setCurrentIndex( static_cast< int >( spxConfig->getUnitsWaterType() ) );
    //
    connectSlots( SIGNALS_UNITS );
  }

  /**
   * @brief GUI für Setpoint nach spxConfig einrichten
   */
  void DeviceConfigFragment::setGuiForSetpoint()
  {
    disconnectSlots( SIGNALS_SETPOINT );
    //
    ui->setpointSetpointComboBox->setCurrentIndex( static_cast< int >( spxConfig->getSetpointValue() ) );
    ui->setpointAutoOnComboBox->setCurrentIndex( static_cast< int >( spxConfig->getSetpointAuto() ) );
    //
    connectSlots( SIGNALS_SETPOINT );
  }

  /**
   * @brief GUI für Lizenz nach spxConfig einrichten
   */
  void DeviceConfigFragment::setGuiForLicense()
  {
    SPX42License lic( spxConfig->getLicense() );

    lg->debug( QString( "DeviceConfigFragment::licChangedSlot -> set: %1" ).arg( static_cast< int >( lic.getLicType() ) ) );
    ui->tabHeaderLabel->setText(
        QString( configHeadlineTemplate.arg( spxConfig->getSerialNumber() ).arg( spxConfig->getLicName() ) ) );
    if ( lic.getLicInd() == IndividualLicense::LIC_INDIVIDUAL )
    {
      // der darf das!
      ui->individualVerticalWidget->setEnabled( true );
      ui->individualVerticalWidget->show();
    }
    else
    {
      // die Grundeinstellungen (zur Sicherheit)
      ui->individualVerticalWidget->setEnabled( false );
      ui->individualVerticalWidget->hide();
      // Einstellungen neutral
      disconnectSlots( SIGNALS_INDIVIDUAL );
      spxConfig->setIndividualSensorsOn( DeviceIndividualSensors::SENSORS_ON );
      spxConfig->setIndividualPscrMode( DeviceIndividualPSCR::PSCR_OFF );
      spxConfig->setIndividualSensorsCount( DeviceIndividualSensorCount::SENSOR_COUNT_03 );
      spxConfig->setIndividualAcoustic( DeviceIndividualAcoustic::ACOUSTIC_ON );
      spxConfig->setIndividualLogInterval( DeviceIndividualLogInterval::INTERVAL_60 );
      connectSlots( SIGNALS_INDIVIDUAL );
    }
    setGuiForIndividual();
  }

  /**
   * @brief GUI für Individual nach spxConfig einrichten
   */
  void DeviceConfigFragment::setGuiForIndividual()
  {
    if ( spxConfig->getLicense().getLicInd() == IndividualLicense::LIC_INDIVIDUAL )
    {
      disconnectSlots( SIGNALS_INDIVIDUAL );
      //
      if ( spxConfig->getIndividualSensorsOn() == DeviceIndividualSensors::SENSORS_ON )
      {
        ui->individualSeonsorsOnCheckBox->setCheckState( Qt::CheckState::Checked );
      }
      else
      {
        ui->individualSeonsorsOnCheckBox->setCheckState( Qt::CheckState::Unchecked );
      }
      if ( spxConfig->getIndividualPscrMode() == DeviceIndividualPSCR::PSCR_ON )
      {
        ui->individualPSCRModeOnCheckBox->setCheckState( Qt::CheckState::Checked );
      }
      else
      {
        ui->individualPSCRModeOnCheckBox->setCheckState( Qt::CheckState::Unchecked );
      }
      ui->individualSensorsCountComboBox->setCurrentIndex( static_cast< int >( spxConfig->getIndividualSensorsCount() ) );
      if ( spxConfig->getIndividualAcoustic() == DeviceIndividualAcoustic::ACOUSTIC_ON )
      {
        ui->individualAcousticWarningsOnCheckBox->setCheckState( Qt::CheckState::Checked );
      }
      else
      {
        ui->individualAcousticWarningsOnCheckBox->setCheckState( Qt::CheckState::Unchecked );
      }
      ui->individualLogIntervalComboBox->setCurrentIndex( static_cast< int >( spxConfig->getIndividualLogInterval() ) );
      //
      connectSlots( SIGNALS_INDIVIDUAL );
    }
  }

  void DeviceConfigFragment::setGuiConnected( bool connected )
  {
    // GUI online oder offline
    ui->decompressionGroupBox->setEnabled( connected );
    ui->displayGroupBox->setEnabled( connected );
    ui->unitGroupBox->setEnabled( connected );
    ui->setpointGroupBox->setEnabled( connected );
    //
    // individual abhängig von der Lizenz
    //
    if ( ( spxConfig->getLicense().getLicInd() == IndividualLicense::LIC_INDIVIDUAL ) && connected )
      ui->individualGroupBox->setEnabled( true );
    else
      ui->individualGroupBox->setEnabled( false );
    // die Überschrift sicherheitshalber auc noch
    ui->tabHeaderLabel->setText(
        QString( configHeadlineTemplate.arg( spxConfig->getSerialNumber() ).arg( spxConfig->getLicName() ) ) );
  }

  /**
   * @brief trenne Slots von Signalen
   */
  void DeviceConfigFragment::disconnectSlots( quint8 which_signals )
  {
    //
    // Alle Slots trennen
    //
    if ( SIGNALS_INDIVIDUAL & which_signals )
    {
      // INDIVIDUAL
      disconnect( ui->individualLogIntervalComboBox, nullptr, nullptr, nullptr );
      disconnect( ui->individualAcousticWarningsOnCheckBox, nullptr, nullptr, nullptr );
      disconnect( ui->individualSensorsCountComboBox, nullptr, nullptr, nullptr );
      disconnect( ui->individualPSCRModeOnCheckBox, nullptr, nullptr, nullptr );
      disconnect( ui->individualSeonsorsOnCheckBox, nullptr, nullptr, nullptr );
      disconnect( ui->individualTempStickComboBox, nullptr, nullptr, nullptr );
    }

    if ( SIGNALS_SETPOINT & which_signals )
    {
      // SETPOINT
      disconnect( ui->setpointSetpointComboBox, nullptr, nullptr, nullptr );
      disconnect( ui->setpointAutoOnComboBox, nullptr, nullptr, nullptr );
    }

    if ( SIGNALS_UNITS & which_signals )
    {
      // EINHEITEN
      disconnect( ui->unitsWaterTypeComboBox, nullptr, nullptr, nullptr );
      disconnect( ui->unitsDeepComboBox, nullptr, nullptr, nullptr );
      disconnect( ui->unitsTemperaturComboBox, nullptr, nullptr, nullptr );
    }

    if ( SIGNALS_DISPLAY & which_signals )
    {
      // DISPLAY
      disconnect( ui->displayOrientationComboBox, nullptr, nullptr, nullptr );
      disconnect( ui->displayBrightnessComboBox, nullptr, nullptr, nullptr );
    }

    if ( SIGNALS_DECOMPRESSION & which_signals )
    {
      // Dekompression
      disconnect( ui->deepStopOnCheckBox, nullptr, nullptr, nullptr );
      disconnect( ui->dynamicGradientsOnCheckBox, nullptr, nullptr, nullptr );
      disconnect( ui->gradientHighSpinBox, nullptr, nullptr, nullptr );
      disconnect( ui->gradientLowSpinBox, nullptr, nullptr, nullptr );
      disconnect( ui->conservatismComboBox, nullptr, nullptr, nullptr );
      disconnect( ui->lastDecoStopComboBox, nullptr, this, nullptr );
    }

    if ( SIGNALS_PROGRAM & which_signals )
    {
      // Lizenz
      disconnect( spxConfig.get(), nullptr, this, nullptr );
      // CONFIG
      disconnect( remoteSPX42.get(), nullptr, this, nullptr );
    }
  }

  /**
   * @brief DeviceConfigFragment::connectSlots
   */
  void DeviceConfigFragment::connectSlots( quint8 which_signals )
  {
    //
    // Alle Slots verbinden
    //
    if ( SIGNALS_INDIVIDUAL & which_signals )
    {
      // INDIVIDUAL
      connect( ui->individualSeonsorsOnCheckBox, &QCheckBox::stateChanged, this,
               &DeviceConfigFragment::onIndividualSensorsOnChangedSlot );
      connect( ui->individualPSCRModeOnCheckBox, &QCheckBox::stateChanged, this,
               &DeviceConfigFragment::onInIndividualPscrModeChangedSlot );
      connect( ui->individualSensorsCountComboBox, static_cast< void ( QComboBox::* )( int ) >( &QComboBox::currentIndexChanged ),
               this, &DeviceConfigFragment::onIndividualSensorsCountChangedSlot );
      connect( ui->individualAcousticWarningsOnCheckBox, &QCheckBox::stateChanged, this,
               &DeviceConfigFragment::onIndividualAcousticChangedSlot );
      connect( ui->individualLogIntervalComboBox, static_cast< void ( QComboBox::* )( int ) >( &QComboBox::currentIndexChanged ), this,
               &DeviceConfigFragment::onIndividualLogIntervalChangedSlot );
      connect( ui->individualTempStickComboBox, static_cast< void ( QComboBox::* )( int ) >( &QComboBox::currentIndexChanged ), this,
               &DeviceConfigFragment::onIndividualTempstickChangedSlot );
    }

    if ( SIGNALS_SETPOINT & which_signals )
    {
      // SETPOINT
      connect( ui->setpointAutoOnComboBox, static_cast< void ( QComboBox::* )( int ) >( &QComboBox::currentIndexChanged ), this,
               &DeviceConfigFragment::onSetpointAutoChangedSlot );
      connect( ui->setpointSetpointComboBox, static_cast< void ( QComboBox::* )( int ) >( &QComboBox::currentIndexChanged ), this,
               &DeviceConfigFragment::onSetpointValueChangedSlot );
    }

    if ( SIGNALS_UNITS & which_signals )
    {
      // EINHEITEN
      connect( ui->unitsTemperaturComboBox, static_cast< void ( QComboBox::* )( int ) >( &QComboBox::currentIndexChanged ), this,
               &DeviceConfigFragment::onUnitsTemperatureChangedSlot );
      connect( ui->unitsDeepComboBox, static_cast< void ( QComboBox::* )( int ) >( &QComboBox::currentIndexChanged ), this,
               &DeviceConfigFragment::onUnitsLengthChangedSlot );
      connect( ui->unitsWaterTypeComboBox, static_cast< void ( QComboBox::* )( int ) >( &QComboBox::currentIndexChanged ), this,
               &DeviceConfigFragment::onUnitsWatertypeChangedSlot );
    }

    if ( SIGNALS_INDIVIDUAL & which_signals )
    {
      // DISPLAY
      connect( ui->displayBrightnessComboBox, static_cast< void ( QComboBox::* )( int ) >( &QComboBox::currentIndexChanged ), this,
               &DeviceConfigFragment::onDisplayBrightnessChangedSlot );
      connect( ui->displayOrientationComboBox, static_cast< void ( QComboBox::* )( int ) >( &QComboBox::currentIndexChanged ), this,
               &DeviceConfigFragment::onDisplayOrientationChangedSlot );
    }

    if ( SIGNALS_DECOMPRESSION & which_signals )
    {
      // Dekompression
      connect( ui->conservatismComboBox, static_cast< void ( QComboBox::* )( int ) >( &QComboBox::currentIndexChanged ), this,
               &DeviceConfigFragment::onDecoComboChangedSlot );
      connect( ui->gradientLowSpinBox, static_cast< void ( QSpinBox::* )( int ) >( &QSpinBox::valueChanged ), this,
               &DeviceConfigFragment::onDecoGradientLowChangedSlot );
      connect( ui->gradientHighSpinBox, static_cast< void ( QSpinBox::* )( int ) >( &QSpinBox::valueChanged ), this,
               &DeviceConfigFragment::onDecoGradientHighChangedSlot );
      connect( ui->dynamicGradientsOnCheckBox, &QCheckBox::stateChanged, this,
               &DeviceConfigFragment::onDecoDynamicGradientStateChangedSlot );
      connect( ui->deepStopOnCheckBox, &QCheckBox::stateChanged, this, &DeviceConfigFragment::onDecoDeepStopsEnableChangedSlot );
      connect( ui->lastDecoStopComboBox, static_cast< void ( QComboBox::* )( int ) >( &QComboBox::currentIndexChanged ), this,
               &DeviceConfigFragment::onDecoLastStopChangedSlot );
    }

    if ( SIGNALS_PROGRAM & which_signals )
    {
      // Lizenz
      connect( spxConfig.get(), &SPX42Config::licenseChangedSig, this, &DeviceConfigFragment::onConfLicChangedSlot );
      // CONFIG
      connect( remoteSPX42.get(), &SPX42RemotBtDevice::onStateChangedSig, this, &DeviceConfigFragment::onOnlineStatusChangedSlot );
      connect( remoteSPX42.get(), &SPX42RemotBtDevice::onSocketErrorSig, this, &DeviceConfigFragment::onSocketErrorSlot );
      connect( remoteSPX42.get(), &SPX42RemotBtDevice::onCommandRecivedSig, this, &DeviceConfigFragment::onCommandRecivedSlot );
    }
  }

  /**
   * @brief changeEvent Signal vom System bearbeiten
   * @param Event
   */
  void DeviceConfigFragment::changeEvent( QEvent *e )
  {
    QWidget::changeEvent( e );
    switch ( e->type() )
    {
      case QEvent::LanguageChange:
        ui->retranslateUi( this );
        break;
      default:
        break;
    }
  }

  /**
   * @brief Slot beim ändern der Lizenz
   * @param Lizenz ders SPX
   */
  void DeviceConfigFragment::onConfLicChangedSlot()
  {
    setGuiForLicense();
  }

  void DeviceConfigFragment::onOnlineStatusChangedSlot( bool )
  {
    setGuiConnected( ( remoteSPX42->getConnectionStatus() == SPX42RemotBtDevice::SPX42_CONNECTED ) ||
                     remoteSPX42->getConnectionStatus() == SPX42RemotBtDevice::SPX42_CONNECTING );

    //
    // wenn der SPX gerade verbunden wurde, frag ihn nach den Angaben
    // aber ein wenig warten, damit andere Prozesse auch eine Chance haben
    //
    if ( remoteSPX42->getConnectionStatus() == SPX42RemotBtDevice::SPX42_CONNECTED )
      QTimer::singleShot( 450, this, &DeviceConfigFragment::onConfigUpdateSlot );
  }

  void DeviceConfigFragment::onSocketErrorSlot( QBluetoothSocket::SocketError )
  {
    // TODO: implementieren
  }

  void DeviceConfigFragment::onCloseDatabaseSlot()
  {
    // TODO: implementieren
  }

  void DeviceConfigFragment::updateGuiFirmwareSpecific()
  {
    //
    // Firmwarespezifische Sachen einstellen
    //
    if ( spxConfig->getIsSixMetersAutoSetpoint() )
    {
      // neuere Firmware, wert von 6, 10, 15 Meter
      // anpassen der GUI
      ui->setpointAutoOnComboBox->clear();
      ui->setpointAutoOnComboBox->addItems( newAutoSetpoint );
    }
    else
    {
      // alte Firmware deact, 6, 10, 15, 20 Meter
      // anpassen der GUI
      ui->setpointAutoOnComboBox->clear();
      ui->setpointAutoOnComboBox->addItems( oldAutoSetpoint );
    }
    if ( spxConfig->getIsNewerDisplayBrightness() )
    {
      // NEU: 0->20%, 1->40%, 2->60%, 3->80%, 4->100%
      ui->displayBrightnessComboBox->clear();
      ui->displayBrightnessComboBox->addItems( newDisplayBrightness );
    }
    else
    {
      // ALT: D= 0->10&, 1->50%, 2->100%
      ui->displayBrightnessComboBox->clear();
      ui->displayBrightnessComboBox->addItems( oldDisplayBrightness );
    }
    //
    // hat das Ding einen Tempstick?
    //
    ui->individualTempStickComboBox->setVisible( spxConfig->getHasSixValuesIndividual() );
    ui->individualTempStickComboBox->setEnabled( spxConfig->getHasSixValuesIndividual() );
    ui->individualTempStickLabel->setVisible( spxConfig->getHasSixValuesIndividual() );
  }

  void DeviceConfigFragment::onCommandRecivedSlot()
  {
    spSingleCommand recCommand;
    QDateTime nowDateTime;
    QByteArray value;
    DecoGradient currentGradient;
    DecompressionPreset preset;

    char kdo;
    //
    lg->debug( "DeviceConfigFragment::onDatagramRecivedSlot..." );
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
          lg->debug( "DeviceConfigFragment::onDatagramRecivedSlot -> alive/acku..." );
          ackuVal = ( recCommand->getValueAt( SPXCmdParam::ALIVE_POWER ) / 100.0 );
          emit onAkkuValueChangedSig( ackuVal );
          break;
        case SPX42CommandDef::SPX_APPLICATION_ID:
          // Kommando APPLICATION_ID (VERSION)
          // ~04:NR -> VERSION als String
          lg->debug( "DeviceConfigFragment::onDatagramRecivedSlot -> firmwareversion..." );
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
          updateGuiFirmwareSpecific();
          break;
        case SPX42CommandDef::SPX_SERIAL_NUMBER:
          // Kommando SERIAL NUMBER
          // ~07:XXX -> Seriennummer als String
          lg->debug( "DeviceConfigFragment::onDatagramRecivedSlot -> serialnumber..." );
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
          lg->debug( "DeviceConfigFragment::onDatagramRecivedSlot -> license state..." );
          spxConfig->setLicense( recCommand->getParamAt( SPXCmdParam::LICENSE_STATE ),
                                 recCommand->getParamAt( SPXCmdParam::LICENSE_INDIVIDUAL ) );
          spxConfig->freezeConfigs( SPX42ConfigClass::CF_CLASS_SPX );
          setGuiConnected( remoteSPX42->getConnectionStatus() == SPX42RemotBtDevice::SPX42_CONNECTED );
          break;
        case SPX42CommandDef::SPX_GET_SETUP_DEKO:
          lg->debug( "DeviceConfigFragment::onDatagramRecivedSlot -> deco state..." );
          // Kommando DEC liefert zurück:
          // ~34:LL:HH:D:Y:C
          // LL=GF-Low, HH=GF-High,
          // D=Deepstops (0/1)
          // Y=Dynamische Gradienten (0/1)
          // C=Last Decostop (0=3 Meter/1=6 Meter)
          //
          // eintrag in GUI -> durch callback dann auch eintrag in config
          lg->debug( QString( "DeviceConfigFragment::onDatagramRecivedSlot -> LOW: %1 HIGH: %2" )
                         .arg( recCommand->getValueAt( SPXCmdParam::DECO_GF_LOW ) )
                         .arg( recCommand->getValueAt( SPXCmdParam::DECO_GF_HIGH ) ) );
          // config speichern
          disconnectSlots( SIGNALS_DECOMPRESSION );
          currentGradient.first = static_cast< qint8 >( recCommand->getValueAt( SPXCmdParam::DECO_GF_LOW ) );
          currentGradient.second = static_cast< qint8 >( recCommand->getValueAt( SPXCmdParam::DECO_GF_HIGH ) );
          preset = spxConfig->getPresetForGradient( currentGradient.first, currentGradient.second );
          spxConfig->setCurrentPreset( preset, currentGradient.first, currentGradient.second );
          spxConfig->setIsDeepstopsEnabled( recCommand->getValueAt( SPXCmdParam::DECO_DEEPSTOPS ) == 1
                                                ? DecompressionDeepstops::DEEPSTOPS_ENABLED
                                                : DecompressionDeepstops::DEEPSTOPS_DISABLED );
          spxConfig->setIsDecoDynamicGradients( recCommand->getValueAt( SPXCmdParam::DECO_DYNGRADIENTS ) == 1
                                                    ? DecompressionDynamicGradient::DYNAMIC_GRADIENT_ON
                                                    : DecompressionDynamicGradient::DYNAMIC_GRADIENT_OFF );
          // GUI einrichten
          ui->gradientLowSpinBox->setValue( static_cast< int >( currentGradient.first ) );
          ui->gradientHighSpinBox->setValue( static_cast< int >( currentGradient.second ) );
          ui->conservatismComboBox->setCurrentIndex( static_cast< int >( preset ) );
          ui->deepStopOnCheckBox->setCheckState(
              recCommand->getValueAt( SPXCmdParam::DECO_DEEPSTOPS ) == 1 ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );
          ui->dynamicGradientsOnCheckBox->setCheckState(
              recCommand->getValueAt( SPXCmdParam::DECO_DYNGRADIENTS ) == 1 ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );
          ui->lastDecoStopComboBox->setCurrentIndex( recCommand->getValueAt( SPXCmdParam::DECO_LASTSTOP ) == 1 ? 1 : 0 );
          spxConfig->freezeConfigs( SPX42ConfigClass::CF_CLASS_DECO );
          connectSlots( SIGNALS_DECOMPRESSION );
          break;
        case SPX42CommandDef::SPX_GET_SETUP_SETPOINT:
          lg->debug( "DeviceConfigFragment::onDatagramRecivedSlot -> setpoint..." );
          // Kommando GET_SETUP_SETPOINT liefert
          // ~35:A:P
          // A = Setpoint bei alte Firmware (0,1,2,3) = (deact,6,10,15,20)
          // A = Setpoint bei neue firmware (0,1,2) = (6,10,15)
          // P = Partialdruck (0..4) 1.0 .. 1.4
          lg->debug( QString( "DeviceConfigFragment::onDatagramRecivedSlot -> autosetpoint %1, setpoint 1.%2..." )
                         .arg( recCommand->getValueAt( SPXCmdParam::SETPOINT_AUTO ) )
                         .arg( recCommand->getValueAt( SPXCmdParam::SETPOINT_VALUE ) ) );
          disconnectSlots( SIGNALS_SETPOINT );
          // config speichern
          spxConfig->setSetpointAuto( static_cast< DeviceSetpointAuto >( recCommand->getValueAt( SPXCmdParam::SETPOINT_AUTO ) ) );
          spxConfig->setSetpointValue( static_cast< DeviceSetpointValue >( recCommand->getValueAt( SPXCmdParam::SETPOINT_VALUE ) ) );
          // GUI machen
          switch ( recCommand->getValueAt( SPXCmdParam::SETPOINT_AUTO ) )
          {
            // autosetpoint bei Tiefe....
            default:
            case 0:
              ui->setpointAutoOnComboBox->setCurrentIndex( 0 );
              break;
            case 1:
              ui->setpointAutoOnComboBox->setCurrentIndex( 1 );
              break;
            case 2:
              ui->setpointAutoOnComboBox->setCurrentIndex( 2 );
              break;
            case 3:
              ui->setpointAutoOnComboBox->setCurrentIndex( 3 );
              break;
            case 4:
              ui->setpointAutoOnComboBox->setCurrentIndex( 4 );
              break;
          }
          switch ( recCommand->getValueAt( SPXCmdParam::SETPOINT_VALUE ) )
          {
            // setpoint wert 1.0 bis 1.4
            default:
            case 0:
              ui->setpointSetpointComboBox->setCurrentIndex( 0 );
              break;
            case 1:
              ui->setpointSetpointComboBox->setCurrentIndex( 1 );
              break;
            case 2:
              ui->setpointSetpointComboBox->setCurrentIndex( 2 );
              break;
            case 3:
              ui->setpointSetpointComboBox->setCurrentIndex( 3 );
              break;
            case 4:
              ui->setpointSetpointComboBox->setCurrentIndex( 4 );
              break;
          }
          spxConfig->freezeConfigs( SPX42ConfigClass::CF_CLASS_SETPOINT );
          connectSlots( SIGNALS_SETPOINT );
          break;
        case SPX42CommandDef::SPX_GET_SETUP_DISPLAYSETTINGS:
          // Kommando GET_SETUP_DISPLAYSETTINGS liefert
          // ~36:D:A
          // ALT: D= 0->10&, 1->50%, 2->100%
          // NEU: 0->20%, 1->40%, 2->60%, 3->80%, 4->100%
          // A= 0->Landscape 1->180Grad
          lg->debug( QString( "DeviceConfigFragment::onDatagramRecivedSlot -> display settings bright: %1 orient: %2..." )
                         .arg( recCommand->getValueAt( SPXCmdParam::DISPLAY_BRIGHTNESS ) )
                         .arg( recCommand->getValueAt( SPXCmdParam::DISPLAY_ORIENT ) ) );
          disconnectSlots( SIGNALS_DISPLAY );
          // config machen
          spxConfig->setDisplayBrightness(
              static_cast< DisplayBrightness >( recCommand->getValueAt( SPXCmdParam::DISPLAY_BRIGHTNESS ) ) );
          spxConfig->setDisplayOrientation(
              static_cast< DisplayOrientation >( recCommand->getValueAt( SPXCmdParam::DISPLAY_ORIENT ) ) );
          // GUI
          if ( spxConfig->getIsNewerDisplayBrightness() )
          {
            switch ( recCommand->getValueAt( SPXCmdParam::DISPLAY_BRIGHTNESS ) )
            {
              default:
              case 0:
                ui->displayBrightnessComboBox->setCurrentIndex( 0 );
                break;
              case 1:
                ui->displayBrightnessComboBox->setCurrentIndex( 1 );
                break;
              case 2:
                ui->displayBrightnessComboBox->setCurrentIndex( 2 );
                break;
              case 3:
                ui->displayBrightnessComboBox->setCurrentIndex( 3 );
                break;
              case 4:
                ui->displayBrightnessComboBox->setCurrentIndex( 4 );
                break;
            }
          }
          else
          {
            switch ( recCommand->getValueAt( SPXCmdParam::DISPLAY_BRIGHTNESS ) )
            {
              default:
              case 0:
                ui->displayBrightnessComboBox->setCurrentIndex( 0 );
                break;
              case 1:
                ui->displayBrightnessComboBox->setCurrentIndex( 1 );
                break;
              case 2:
                ui->displayBrightnessComboBox->setCurrentIndex( 2 );
                break;
            }
          }
          switch ( recCommand->getValueAt( SPXCmdParam::DISPLAY_ORIENT ) )
          {
            default:
            case 0:
              ui->displayOrientationComboBox->setCurrentIndex( 0 );
              break;
            case 1:
              ui->displayOrientationComboBox->setCurrentIndex( 1 );
              break;
          }
          spxConfig->freezeConfigs( SPX42ConfigClass::CF_CLASS_DISPLAY );
          connectSlots( SIGNALS_DISPLAY );
          break;
        case SPX42CommandDef::SPX_GET_SETUP_UNITS:
          // Kommando GET_SETUP_UNITS
          // ~37:UD:UL:UW
          // UD= Fahrenheit/Celsius => immer 0 in der Firmware 2.6.7.7_U
          // UL= 0=metrisch 1=imperial
          // UW= 0->Salzwasser 1->Süßwasser
          lg->debug( QString( "DeviceConfigFragment::onDatagramRecivedSlot -> unit settings temp: %1 dimen: %2 water: %3..." )
                         .arg( recCommand->getValueAt( SPXCmdParam::UNITS_TEMPERATURE ) )
                         .arg( recCommand->getValueAt( SPXCmdParam::UNITS_METRIC_OR_IMPERIAL ) )
                         .arg( recCommand->getValueAt( SPXCmdParam::UNITS_SALT_OR_FRESHWATER ) ) );
          disconnectSlots( SIGNALS_UNITS );
          // CONFIG
          spxConfig->setUnitsTemperatur(
              static_cast< DeviceTemperaturUnit >( recCommand->getValueAt( SPXCmdParam::UNITS_TEMPERATURE ) ) );
          spxConfig->setUnitsLength(
              static_cast< DeviceLenghtUnit >( recCommand->getValueAt( SPXCmdParam::UNITS_METRIC_OR_IMPERIAL ) ) );
          spxConfig->setUnitsWaterType(
              static_cast< DeviceWaterType >( recCommand->getValueAt( SPXCmdParam::UNITS_SALT_OR_FRESHWATER ) ) );
          // GUI
          switch ( recCommand->getValueAt( SPXCmdParam::UNITS_TEMPERATURE ) )
          {
            // Celsios oder Fahrenheid?
            default:
            case 0:
              ui->unitsTemperaturComboBox->setCurrentIndex( 0 );
              break;
            case 1:
              ui->unitsTemperaturComboBox->setCurrentIndex( 1 );
          }
          switch ( recCommand->getValueAt( SPXCmdParam::UNITS_METRIC_OR_IMPERIAL ) )
          {
            // Metrisch oder imperial
            default:
            case 0:
              ui->unitsDeepComboBox->setCurrentIndex( 0 );
              break;
            case 1:
              if ( spxConfig->getHasFahrenheidBug() )
              {
                // bei der alten Firmware den Workarround =>
                // macht dass die Einstellungen immer korrespondieren
                ui->unitsTemperaturComboBox->setCurrentIndex( 1 );
              }
              ui->unitsDeepComboBox->setCurrentIndex( 1 );
          }
          switch ( recCommand->getValueAt( SPXCmdParam::UNITS_SALT_OR_FRESHWATER ) )
          {
            // süß oder Salzwasser
            default:
            case 0:
              ui->unitsWaterTypeComboBox->setCurrentIndex( 0 );
              break;
            case 1:
              ui->unitsWaterTypeComboBox->setCurrentIndex( 1 );
              break;
          }
          spxConfig->freezeConfigs( SPX42ConfigClass::CF_CLASS_UNITS );
          connectSlots( SIGNALS_UNITS );
          break;
        case SPX42CommandDef::SPX_GET_SETUP_INDIVIDUAL:
          lg->debug(
              QString( "DeviceConfigFragment::onDatagramRecivedSlot -> individual: se: %1 pscr: %2 sc: %3 snd: %4 li:%5 ??: %6..." )
                  .arg( recCommand->getValueAt( SPXCmdParam::INDIVIDUAL_SENSORSENABLED ) )
                  .arg( recCommand->getValueAt( SPXCmdParam::INDIVIDUAL_PASSIVEMODE ) )
                  .arg( recCommand->getValueAt( SPXCmdParam::INDIVIDUAL_SENSORCOUNT ) )
                  .arg( recCommand->getValueAt( SPXCmdParam::INDIVIDUAL_SOUND_ONOFF ) )
                  .arg( recCommand->getValueAt( SPXCmdParam::INDIVIDUAL_LOGINTERVAL ) )
                  .arg( recCommand->getValueAt( SPXCmdParam::INDIVIDUAL_TEMPSTICK ) ) );
          // Kommando GET_SETUP_INDIVIDUAL liefert
          // ~38:SE:PS:SC:SN:LI:TS
          // SE: Sensors 0->ON 1->OFF
          // PS: PSCRMODE 0->OFF 1->ON
          // SC: SensorCount 0=>1 1=>2 2=>3
          // SN: Sound 0->OFF 1->ON
          // LI: Loginterval 0->10sec 1->30Sec 2->60 Sec
          // TS: ab Version 2.7_H_r83 Tempstick Version
          //
          disconnectSlots( SIGNALS_INDIVIDUAL );
          // CONFIG einstellen
          spxConfig->setIndividualSensorsOn(
              static_cast< DeviceIndividualSensors >( recCommand->getValueAt( SPXCmdParam::INDIVIDUAL_SENSORSENABLED ) ) );
          spxConfig->setIndividualPscrMode(
              static_cast< DeviceIndividualPSCR >( recCommand->getValueAt( SPXCmdParam::INDIVIDUAL_PASSIVEMODE ) ) );
          spxConfig->setIndividualSensorsCount(
              static_cast< DeviceIndividualSensorCount >( recCommand->getValueAt( SPXCmdParam::INDIVIDUAL_SENSORCOUNT ) ) );
          spxConfig->setIndividualAcoustic(
              static_cast< DeviceIndividualAcoustic >( recCommand->getValueAt( SPXCmdParam::INDIVIDUAL_SOUND_ONOFF ) ) );
          spxConfig->setIndividualLogInterval(
              static_cast< DeviceIndividualLogInterval >( recCommand->getValueAt( SPXCmdParam::INDIVIDUAL_LOGINTERVAL ) ) );
          spxConfig->setIndividualTempStick(
              static_cast< DeviceIndividualTempstick >( recCommand->getValueAt( SPXCmdParam::INDIVIDUAL_TEMPSTICK ) ) );
          // GUI
          // sensor enable
          if ( recCommand->getValueAt( SPXCmdParam::INDIVIDUAL_SENSORSENABLED ) )
            ui->individualSeonsorsOnCheckBox->setCheckState( Qt::CheckState::Unchecked );
          else
            ui->individualSeonsorsOnCheckBox->setCheckState( Qt::CheckState::Checked );
          // pscr mode
          if ( recCommand->getValueAt( SPXCmdParam::INDIVIDUAL_PASSIVEMODE ) )
            ui->individualPSCRModeOnCheckBox->setCheckState( Qt::CheckState::Checked );
          else
            ui->individualPSCRModeOnCheckBox->setCheckState( Qt::CheckState::Unchecked );
          // sensor count
          switch ( recCommand->getValueAt( SPXCmdParam::INDIVIDUAL_SENSORCOUNT ) )
          {
            case 0:
              ui->individualSensorsCountComboBox->setCurrentIndex( 0 );
              break;
            case 1:
              ui->individualSensorsCountComboBox->setCurrentIndex( 1 );
              break;
            case 2:
            default:
              ui->individualSensorsCountComboBox->setCurrentIndex( 2 );
              break;
          }
          // sound warnings
          if ( recCommand->getValueAt( SPXCmdParam::INDIVIDUAL_SOUND_ONOFF ) )
            ui->individualAcousticWarningsOnCheckBox->setCheckState( Qt::CheckState::Checked );
          else
            ui->individualAcousticWarningsOnCheckBox->setCheckState( Qt::CheckState::Unchecked );
          // log interval
          switch ( recCommand->getValueAt( SPXCmdParam::INDIVIDUAL_LOGINTERVAL ) )
          {
            case 0:
              ui->individualLogIntervalComboBox->setCurrentIndex( 0 );
              break;
            case 1:
              ui->individualLogIntervalComboBox->setCurrentIndex( 1 );
              break;
            case 2:
            default:
              ui->individualLogIntervalComboBox->setCurrentIndex( 2 );
              break;
          }
          // tempstick
          switch ( recCommand->getValueAt( SPXCmdParam::INDIVIDUAL_TEMPSTICK ) )
          {
            case 0:
            default:
              ui->individualTempStickComboBox->setCurrentIndex( 0 );
              break;
            case 1:
              ui->individualTempStickComboBox->setCurrentIndex( 1 );
              break;
            case 3:
              ui->individualTempStickComboBox->setCurrentIndex( 2 );
              break;
          }
          spxConfig->freezeConfigs( SPX42ConfigClass::CF_CLASS_INDIVIDUAL );
          connectSlots( SIGNALS_INDIVIDUAL );
          break;
      }
      //
      // falls es mehr gibt, lass dem Rest der App auch eine Chance
      //
      QCoreApplication::processEvents();
    }
  }

  /**
   * @brief Dekompressionseinstellung geändert
   * @param der neue Index
   */
  void DeviceConfigFragment::onDecoComboChangedSlot( int index )
  {
    DecoGradient newGradient;
    //
    if ( gradentSlotsIgnore )
      return;
    //
    // index korrespondiert mit dem Key des Deko-Presets
    // QHash aus SPX42Config
    //
    newGradient = spxConfig->getPresetValues( static_cast< DecompressionPreset >( index ) );
    lg->debug( QString( "DeviceConfigFragment::onDecoComboChangedSlot: combobox new index: %1, new gradients low: %2, high: %3 " )
                   .arg( index )
                   .arg( newGradient.first )
                   .arg( newGradient.second ) );
    //
    // Slots für changeInput kurz deaktivieren
    // und dann Werte neu eintragen
    //
    gradentSlotsIgnore = true;
    ui->gradientLowSpinBox->setValue( newGradient.first );
    ui->gradientHighSpinBox->setValue( newGradient.second );
    spxConfig->setCurrentPreset( static_cast< DecompressionPreset >( index ), newGradient.first, newGradient.second );
    gradentSlotsIgnore = false;
    emit onConfigWasChangedSig();
  }

  /**
   * @brief der LOW Gradient wurde manuell geändert
   * @param der neue LOW Wert
   */
  void DeviceConfigFragment::onDecoGradientLowChangedSlot( int _low )
  {
    DecompressionPreset preset;
    // DecoGradient newGradient;
    qint8 high;
    qint8 low = static_cast< qint8 >( _low );
    //
    if ( gradentSlotsIgnore )
      return;
    high = static_cast< qint8 >( ui->gradientHighSpinBox->value() );
    lg->debug( QString( "DeviceConfigFragment::onDecoGradientLowChangedSlot: gradients changed to low: %1 high: %2" )
                   .arg( static_cast< int >( low ) )
                   .arg( static_cast< int >( high ) ) );
    //
    // Passen die Werte zu einem Preset?
    //
    preset = spxConfig->getPresetForGradient( low, high );
    //
    // hat sich dabei das Preset verändert?
    //
    if ( ui->conservatismComboBox->currentIndex() != static_cast< int >( preset ) )
    {
      setGradientPresetWithoutCallback( preset );
    }
    spxConfig->setCurrentPreset( preset, low, high );
    emit onConfigWasChangedSig();
  }

  /**
   * @brief der HIGH Gradient wurde manuell geändert
   * @param der neue HIGH Wert
   */
  void DeviceConfigFragment::onDecoGradientHighChangedSlot( int _high )
  {
    DecompressionPreset preset;
    // DecoGradient newGradient;
    qint8 low;
    qint8 high = static_cast< qint8 >( _high );
    //
    if ( gradentSlotsIgnore )
      return;
    low = static_cast< qint8 >( ui->gradientLowSpinBox->value() );
    lg->debug( QString( "DeviceConfigFragment::onDecoGradientHighChangedSlot: gradients changed to low: %1 high: %2" )
                   .arg( static_cast< int >( low ) )
                   .arg( static_cast< int >( high ) ) );
    //
    // Passen die Werte zu einem Preset?
    //
    preset = spxConfig->getPresetForGradient( low, high );
    if ( ui->conservatismComboBox->currentIndex() != static_cast< int >( preset ) )
    {
      setGradientPresetWithoutCallback( preset );
    }
    spxConfig->setCurrentPreset( preset, low, high );
    emit onConfigWasChangedSig();
  }

  /**
   * @brief Hilfsfunktion zum ändern des Presets in der Dropdown ohne Callback auszulösen
   * @param der NEUE Preset
   */
  void DeviceConfigFragment::setGradientPresetWithoutCallback( DecompressionPreset preset )
  {
    //
    // zuerst Callbacks ignorieren, sondst gibt das eine Endlosschleife
    //
    gradentSlotsIgnore = true;
    ui->conservatismComboBox->setCurrentIndex( static_cast< int >( preset ) );
    lg->debug( QString( "DeviceConfigFragment::setGradientPresetWithoutCallback: combobox new index: %1" )
                   .arg( static_cast< int >( preset ) ) );
    //
    // Callbacks wieder erlauben
    //
    gradentSlotsIgnore = false;
  }

  /**
   * @brief Dynamiche gradienten ein/aus schalten
   * @param state neuer Status
   */
  void DeviceConfigFragment::onDecoDynamicGradientStateChangedSlot( int state )
  {
    DecompressionDynamicGradient newState = DecompressionDynamicGradient::DYNAMIC_GRADIENT_OFF;
    //
    lg->debug( QString( "DeviceConfigFragment::onDecoDynamicGradientStateChangedSlot: changed to %1" )
                   .arg( state == static_cast< int >( Qt::CheckState::Checked ) ? "ENABLED" : "DISABLED" ) );
    if ( state == static_cast< int >( Qt::CheckState::Checked ) )
      newState = DecompressionDynamicGradient::DYNAMIC_GRADIENT_ON;
    spxConfig->setIsDecoDynamicGradients( newState );
    emit onConfigWasChangedSig();
  }

  /**
   * @brief Tiefe Deco-Stops ein/aus
   * @param state neuer Status
   */
  void DeviceConfigFragment::onDecoDeepStopsEnableChangedSlot( int state )
  {
    DecompressionDeepstops newState = DecompressionDeepstops::DEEPSTOPS_DISABLED;
    //
    lg->debug( QString( "DeviceConfigFragment::onDecoDeepStopsEnableChangedSlot: changed to %1" )
                   .arg( state == static_cast< int >( Qt::CheckState::Checked ) ? "ENABLED" : "DISABLED" ) );
    if ( state == static_cast< int >( Qt::CheckState::Checked ) )
      newState = DecompressionDeepstops::DEEPSTOPS_ENABLED;
    spxConfig->setIsDeepstopsEnabled( newState );
    emit onConfigWasChangedSig();
  }

  /**
   * @brief DeviceConfigFragment::onDecoLastStopChangedSlot
   * @param index
   */
  void DeviceConfigFragment::onDecoLastStopChangedSlot( int index )
  {
    // index 0 ==> 3 Meter
    // index 1 ==> 6 Meter
    switch ( index )
    {
      default:
        spxConfig->setLastDecoStop( DecoLastStop::LAST_STOP_ON_3 );
        break;
      case 1:
        spxConfig->setLastDecoStop( DecoLastStop::LAST_STOP_ON_6 );
    }
    emit onConfigWasChangedSig();
  }

  /**
   * @brief Einstellung für Helligkeit des SPX42 Displays
   * @param index INdex der neuen Einstellung
   */
  void DeviceConfigFragment::onDisplayBrightnessChangedSlot( int index )
  {
    //
    if ( lg->getThreshold() == LgThreshold::LG_DEBUG )
    {
      //
      // das nur im debugging Modus machen
      //
      int brVal = 0;
      switch ( index )
      {
        case static_cast< int >( DisplayBrightness::BRIGHT_20 ):
          brVal = 20;
          break;
        case static_cast< int >( DisplayBrightness::BRIGHT_40 ):
          brVal = 40;
          break;
        case static_cast< int >( DisplayBrightness::BRIGHT_60 ):
          brVal = 60;
          break;
        case static_cast< int >( DisplayBrightness::BRIGHT_80 ):
          brVal = 80;
          break;
        case static_cast< int >( DisplayBrightness::BRIGHT_100 ):
        default:
          brVal = 100;
      }
      lg->debug( QString( "DeviceConfigFragment::onDisplayBrightnessChangedSlot: set brightness to %1" ).arg( brVal ) );
    }
    //
    // setze im config-Objekt die Helligkeit
    //
    spxConfig->setDisplayBrightness( static_cast< DisplayBrightness >( index ) );
    emit onConfigWasChangedSig();
  }

  /**
   * @brief Änderung der Ausrichtung des Displays
   * @param index der Displayorientierung
   */
  void DeviceConfigFragment::onDisplayOrientationChangedSlot( int index )
  {
    auto orientation = static_cast< DisplayOrientation >( index );
    if ( lg->getThreshold() == LgThreshold::LG_DEBUG )
    {
      QString orString;
      //
      if ( orientation == DisplayOrientation::LANDSCAPE )
      {
        orString = "landscape";
      }
      else
      {
        orString = "landscape 180°";
      }
      lg->debug( QString( "DeviceConfigFragment::onDisplayOrientationChangedSlot: orientation to %1" ).arg( orString ) );
    }
    spxConfig->setDisplayOrientation( static_cast< DisplayOrientation >( index ) );
    emit onConfigWasChangedSig();
  }

  /**
   * @brief Änderung der Temperatureinheit
   * @param index neuer Index der Einheit
   */
  void DeviceConfigFragment::onUnitsTemperatureChangedSlot( int index )
  {
    auto tUnit = static_cast< DeviceTemperaturUnit >( index );
    if ( lg->getThreshold() == LgThreshold::LG_DEBUG )
    {
      QString tmpString;
      //
      if ( tUnit == DeviceTemperaturUnit::CELSIUS )
      {
        tmpString = "celsius";
      }
      else
      {
        tmpString = "fahrenheid";
      }
      lg->debug( QString( "DeviceConfigFragment::onUnitsTemperatureChangedSlot: temperatur unit to %1" ).arg( tmpString ) );
    }
    spxConfig->setUnitsTemperatur( tUnit );
    emit onConfigWasChangedSig();
  }

  /**
   * @brief Änderung der Längeneinheit
   * @param index neuer Index der Einheit
   */
  void DeviceConfigFragment::onUnitsLengthChangedSlot( int index )
  {
    auto lUnit = static_cast< DeviceLenghtUnit >( index );
    if ( lg->getThreshold() == LgThreshold::LG_DEBUG )
    {
      QString lenString;
      //
      if ( lUnit == DeviceLenghtUnit::METRIC )
      {
        lenString = "metric";
      }
      else
      {
        lenString = "imperial";
      }
      lg->debug( QString( "DeviceConfigFragment::onUnitsLengthChangedSlot: lengt unit to %1" ).arg( lenString ) );
    }
    spxConfig->setUnitsLength( lUnit );
    emit onConfigWasChangedSig();
  }

  /**
   * @brief DeviceConfigFragment::unitsWatertypeChangedSlot
   * @param index
   */
  void DeviceConfigFragment::onUnitsWatertypeChangedSlot( int index )
  {
    if ( lg->getThreshold() == LgThreshold::LG_DEBUG )
    {
      QString typeString;
      auto wUnit = static_cast< DeviceWaterType >( index );
      //
      if ( wUnit == DeviceWaterType::FRESHWATER )
      {
        typeString = "freshwater";
      }
      else
      {
        typeString = "saltwater";
      }
      lg->debug( QString( "DeviceConfigFragment::onUnitsWatertypeChangedSlot: lengt unit to %1" ).arg( typeString ) );
    }
    spxConfig->setUnitsWaterType( static_cast< DeviceWaterType >( index ) );
    emit onConfigWasChangedSig();
    lg->debug( QString( "######## UNIT HASHES: %1" ).arg( spxConfig->geteUnitHashes() ) );
  }

  /**
   * @brief Änderung des Auto setpoints
   * @param index neuer index des Autosetpoints
   */
  void DeviceConfigFragment::onSetpointAutoChangedSlot( int index )
  {
    // TODO: imperiale Maße auch berücksichtigen
    auto autoSP = static_cast< DeviceSetpointAuto >( index );
    if ( lg->getThreshold() == LgThreshold::LG_DEBUG )
    {
      int autoDepth;
      switch ( index )
      {
        case static_cast< int >( DeviceSetpointAuto::AUTO_06 ):
          autoDepth = 6;
          break;
        case static_cast< int >( DeviceSetpointAuto::AUTO_10 ):
          autoDepth = 10;
          break;
        case static_cast< int >( DeviceSetpointAuto::AUTO_15 ):
          autoDepth = 15;
          break;
        case static_cast< int >( DeviceSetpointAuto::AUTO_20 ):
          autoDepth = 20;
          break;
        case static_cast< int >( DeviceSetpointAuto::AUTO_OFF ):
        default:
          autoDepth = 0;
      }
      lg->debug( QString( "DeviceConfigFragment::onSetpointAutoChangedSlot: auto setpoint to %1" ).arg( autoDepth ) );
    }
    spxConfig->setSetpointAuto( autoSP );
    emit onConfigWasChangedSig();
  }

  /**
   * @brief Änderung des O2-Setpoints
   * @param index index des neuen Wertes für Setpoint
   */
  void DeviceConfigFragment::onSetpointValueChangedSlot( int index )
  {
    // TODO: imperiale Maße auch berücksichtigen
    auto setpointValue = static_cast< DeviceSetpointValue >( index );
    if ( lg->getThreshold() == LgThreshold::LG_DEBUG )
    {
      int setpoint;
      switch ( index )
      {
        case static_cast< int >( DeviceSetpointValue::SETPOINT_10 ):
          setpoint = 10;
          break;
        case static_cast< int >( DeviceSetpointValue::SETPOINT_11 ):
          setpoint = 11;
          break;
        case static_cast< int >( DeviceSetpointValue::SETPOINT_12 ):
          setpoint = 12;
          break;
        case static_cast< int >( DeviceSetpointValue::SETPOINT_13 ):
          setpoint = 13;
          break;
        case static_cast< int >( DeviceSetpointValue::SETPOINT_14 ):
        default:
          setpoint = 14;
      }
      lg->debug( QString( "DeviceConfigFragment::onSetpointValueChangedSlot: setpoint (ppo2) to %1" ).arg( setpoint ) );
    }
    spxConfig->setSetpointValue( setpointValue );
    emit onConfigWasChangedSig();
  }

  /**
   * @brief Sensoren an/aus
   * @param state neuer Status
   */
  void DeviceConfigFragment::onIndividualSensorsOnChangedSlot( int state )
  {
    DeviceIndividualSensors newState = DeviceIndividualSensors::SENSORS_OFF;
    //
    lg->debug( QString( "DeviceConfigFragment::onIndividualSensorsOnChangedSlot: changed to %1" ).arg( state ) );
    if ( state == static_cast< int >( Qt::CheckState::Checked ) )
      newState = DeviceIndividualSensors::SENSORS_ON;
    spxConfig->setIndividualSensorsOn( newState );
    emit onConfigWasChangedSig();
  }

  /**
   * @brief Passiv Semiclosed Rebreather Mode ein/aus
   * @param state neuer Status
   */
  void DeviceConfigFragment::onInIndividualPscrModeChangedSlot( int state )
  {
    DeviceIndividualPSCR newState = DeviceIndividualPSCR::PSCR_OFF;
    //
    lg->debug( QString( "DeviceConfigFragment::onInIndividualPscrModeChangedSlot: changed to %1" ).arg( state ) );
    if ( state == static_cast< int >( Qt::CheckState::Checked ) )
      newState = DeviceIndividualPSCR::PSCR_ON;
    spxConfig->setIndividualPscrMode( newState );
    emit onConfigWasChangedSig();
  }

  /**
   * @brief Änderung der Anzahl der genutzten Sensoren
   * @param index index der Anzahl der Sensoren
   */
  void DeviceConfigFragment::onIndividualSensorsCountChangedSlot( int index )
  {
    auto setpointValue = static_cast< DeviceIndividualSensorCount >( index );
    if ( lg->getThreshold() == LgThreshold::LG_DEBUG )
    {
      int sensorCount;
      switch ( index )
      {
        case static_cast< int >( DeviceIndividualSensorCount::SENSOR_COUNT_01 ):
          sensorCount = 1;
          break;
        case static_cast< int >( DeviceIndividualSensorCount::SENSOR_COUNT_02 ):
          sensorCount = 2;
          break;
        case static_cast< int >( DeviceIndividualSensorCount::SENSOR_COUNT_03 ):
        default:
          sensorCount = 3;
      }
      lg->debug( QString( "DeviceConfigFragment::onIndividualSensorsCountChangedSlot: count sensors to %1" ).arg( sensorCount ) );
    }
    spxConfig->setIndividualSensorsCount( setpointValue );
    emit onConfigWasChangedSig();
  }

  /**
   * @brief Änderung der akustischen Meldungen an/aus
   * @param state neuer Status
   */
  void DeviceConfigFragment::onIndividualAcousticChangedSlot( int state )
  {
    DeviceIndividualAcoustic newState = DeviceIndividualAcoustic::ACOUSTIC_OFF;
    //
    lg->debug( QString( "DeviceConfigFragment::onIndividualAcousticChangedSlot: changed to %1" ).arg( state ) );
    if ( state == static_cast< int >( Qt::CheckState::Checked ) )
      newState = DeviceIndividualAcoustic::ACOUSTIC_ON;
    spxConfig->setIndividualAcoustic( newState );
    emit onConfigWasChangedSig();
  }

  /**
   * @brief Änderung des Log intervals
   * @param index index des neuen Wertes
   */
  void DeviceConfigFragment::onIndividualLogIntervalChangedSlot( int index )
  {
    auto logInterval = static_cast< DeviceIndividualLogInterval >( index );
    if ( lg->getThreshold() == LgThreshold::LG_DEBUG )
    {
      int logInterval;
      switch ( index )
      {
        case static_cast< int >( DeviceIndividualLogInterval::INTERVAL_20 ):
          logInterval = 20;
          break;
        case static_cast< int >( DeviceIndividualLogInterval::INTERVAL_30 ):
          logInterval = 30;
          break;
        case static_cast< int >( DeviceIndividualLogInterval::INTERVAL_60 ):
        default:
          logInterval = 60;
      }
      lg->debug( QString( "DeviceConfigFragment::onIndividualLogIntervalChangedSlot: interrval to %1" ).arg( logInterval ) );
    }
    spxConfig->setIndividualLogInterval( logInterval );
    emit onConfigWasChangedSig();
  }

  /**
   * @brief DeviceConfigFragment::onIndividualTempstickChangedSlot
   * @param state
   */
  void DeviceConfigFragment::onIndividualTempstickChangedSlot( int state )
  {
    auto tempstickType = static_cast< DeviceIndividualTempstick >( state );
    spxConfig->setIndividualTempStick( tempstickType );
    lg->debug( QString( "DeviceConfigFragment::onIndividualTempstickChangedSlot -> set type %1" ).arg( state ) );
    emit onConfigWasChangedSig();
  }

  /**
   * @brief DeviceConfigFragment::onConfigUpdateSlot
   */
  void DeviceConfigFragment::onConfigUpdateSlot()
  {
    //
    // es ist Zeit alle Einstellungen abzufragen
    // Seriennummer, Version, Lizenz wird im Objekt
    // SPX42ControlMainWin abgefragt...
    SendListEntry sendCommand = remoteSPX42->askForConfig();
    remoteSPX42->sendCommand( sendCommand );
    lg->debug( "DeviceConfigFragment::onConfigUpdateSlot -> send cmd config..." );
  }
}  // namespace spx
