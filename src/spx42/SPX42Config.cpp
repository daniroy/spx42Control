#include "SPX42Config.hpp"

// TODO: hashes wieder nur bei bedarf machen

namespace spx
{
  /**
   * @brief Standartkonstruktor
   */
  SPX42Config::SPX42Config()
      : QObject( Q_NULLPTR )
      , sendSignals( true )
      , isValid( false )
      , spxFirmwareVersion( SPX42FirmwareVersions::FIRMWARE_UNKNOWN )
      , spxSerialNumber()
      , decoCurrentPreset()
      , decoDynamicGradient()
      , decoDeepstopsEnabled()
      , decoLastStop()
      , displayBrightness()
      , displayOrientation()
      , unitTemperature()
      , unitLength()
      , unitWaterType()
      , setpointAuto()
      , setpointValue()
      , individualSensorsOn()
      , individualPSCROn()
      , individualSensorCount()
      , individualAcustic()
      , individualLogInterval()
      , individualTempStick()
      , hasFahrenheidBug( true )
      , canSetDate( false )
      , hasSixValuesIndividual( false )
      , isFirmwareSupported( false )
      , isOldParamSorting( false )
      , isNewerDisplayBrightness( false )
      , isSixMetersAutoSetpoint( false )
      , qhash( QCryptographicHash::Md5 )
  {
    resetConfig( SPX42ConfigClass::CF_CLASS_ALL );
    // connect( &spxLicense, &SPX42License::licenseChangedPrivateSig, this, &SPX42Config::licenseChangedPrivateSlot );
    //
    // DECO Presets füllen
    //
    decoPresets.insert( static_cast< int >( DecompressionPreset::DECO_KEY_V_CONSERVATIVE ), ProjectConst::DECO_VAL_V_CONSERVATIVE );
    decoPresets.insert( static_cast< int >( DecompressionPreset::DECO_KEY_CONSERVATIVE ), ProjectConst::DECO_VAL_CONSERVATIVE );
    decoPresets.insert( static_cast< int >( DecompressionPreset::DECO_KEY_MODERATE ), ProjectConst::DECO_VAL_MODERATE );
    decoPresets.insert( static_cast< int >( DecompressionPreset::DECO_KEY_AGRESSIVE ), ProjectConst::DECO_VAL_AGRESSIVE );
    decoPresets.insert( static_cast< int >( DecompressionPreset::DECO_KEY_V_AGRESSIVE ), ProjectConst::DECO_VAL_V_AGRESSIVE );
    decoPresets.insert( static_cast< int >( DecompressionPreset::DECO_KEY_CUSTOM ), ProjectConst::DECO_VAL_CUSTOM );
  }

  /**
   * @brief SPX42Config::getSpxFirmwareVersion
   * @return
   */
  SPX42FirmwareVersions SPX42Config::getSpxFirmwareVersion() const
  {
    return spxFirmwareVersion;
  }

  /**
   * @brief SPX42Config::setSpxFirmwareVersion
   * @param value
   */
  void SPX42Config::setSpxFirmwareVersion( SPX42FirmwareVersions value )
  {
    //
    // setzte den Versionswert und setzte Kompatibilitätswerte
    //
    spxFirmwareVersion = value;
    // Voreinstellung für den schlimmsten Fall
    isFirmwareSupported = false;
    hasFahrenheidBug = true;
    canSetDate = false;
    hasSixValuesIndividual = false;
    isOldParamSorting = false;
    isNewerDisplayBrightness = false;
    isSixMetersAutoSetpoint = false;
    //
    // jetzt unterscheiden wer was kann
    //
    switch ( static_cast< qint8 >( value ) )
    {
      case static_cast< qint8 >( SPX42FirmwareVersions::FIRMWARE_UNKNOWN ):
        // der schlimmste Fall :-(
        break;
      case static_cast< qint8 >( SPX42FirmwareVersions::FIRMWARE_2_6x ):
        // eine Firmware 2.6x -> gaaaanz alt
        isFirmwareSupported = true;
        hasFahrenheidBug = true;
        isOldParamSorting = true;
        isSixMetersAutoSetpoint = true;
        break;
      case static_cast< qint8 >( SPX42FirmwareVersions::FIRMWARE_2_7x ):
        // unbestimmte Firmware 2.7xxxx
        isFirmwareSupported = true;
        hasFahrenheidBug = false;
        canSetDate = false;
        hasSixValuesIndividual = false;
        isSixMetersAutoSetpoint = false;
        break;
      case static_cast< qint8 >( SPX42FirmwareVersions::FIRMWARE_2_7_V_R83x ):
        isFirmwareSupported = true;
        // Build 198
        hasFahrenheidBug = false;
        canSetDate = true;
        hasSixValuesIndividual = true;
        isNewerDisplayBrightness = true;
        isSixMetersAutoSetpoint = true;
        break;
      case static_cast< qint8 >( SPX42FirmwareVersions::FIRMWARE_2_7_Hx ):
        isFirmwareSupported = true;
        hasFahrenheidBug = false;
        canSetDate = false;
        break;
      case static_cast< qint8 >( SPX42FirmwareVersions::FIRMWARE_2_7_H_R83x ):
        // Build 197
        isFirmwareSupported = true;
        hasFahrenheidBug = false;
        canSetDate = true;
        hasSixValuesIndividual = true;
        isNewerDisplayBrightness = true;
        isSixMetersAutoSetpoint = true;
        break;
    }
    currentSpxHash = makeSpxHash();
  }

  /**
   * @brief SPX42Config::setSpxFirmwareVersion
   * @param value
   * TODO: neue Firmwareversionen hier einfügen
   */
  void SPX42Config::setSpxFirmwareVersion( const QString &value )
  {
    //
    // vergleiche die im Programm unterstützten Versionen und setzte
    // dann alle Kompatibilitätswerte
    // Reihenfolge ist wichtig, da Suche vom Speziellen zum Allgmeinen läuft
    //
    if ( ProjectConst::FIRMWARE_2_6x.exactMatch( value ) )
      setSpxFirmwareVersion( SPX42FirmwareVersions::FIRMWARE_2_6x );
    // jetzt die 2.7er Versionen
    // exakt die 2.7.H,r83
    else if ( ProjectConst::FIRMWARE_2_7_H_R83x.exactMatch( value ) )
      setSpxFirmwareVersion( SPX42FirmwareVersions::FIRMWARE_2_7_H_R83x );
    // oder 2.7.V.r83
    else if ( ProjectConst::FIRMWARE_2_7_V_R83x.exactMatch( value ) )
      setSpxFirmwareVersion( SPX42FirmwareVersions::FIRMWARE_2_7_V_R83x );
    // oder unbestimmte 2.7H
    else if ( ProjectConst::FIRMWARE_2_7_Hx.exactMatch( value ) )
      setSpxFirmwareVersion( SPX42FirmwareVersions::FIRMWARE_2_7_Hx );
    // oder allgemein eine 2.7x Version
    else if ( ProjectConst::FIRMWARE_2_7x.exactMatch( value ) )
      setSpxFirmwareVersion( SPX42FirmwareVersions::FIRMWARE_2_7x );
    else
      setSpxFirmwareVersion( SPX42FirmwareVersions::FIRMWARE_UNKNOWN );
    currentSpxHash = makeSpxHash();
  }

  /**
   * @brief SPX42Config::setSpxFirmwareVersion
   * @param value
   */
  void SPX42Config::setSpxFirmwareVersion( const QByteArray &value )
  {
    QString versionString( value );
    setSpxFirmwareVersion( versionString );
    currentSpxHash = makeSpxHash();
  }

  /**
   * @brief Gibt die aktuelle SPX42 Lizenz zurück
   * @return Lizenz
   */
  SPX42License &SPX42Config::getLicense()
  {
    return ( spxLicense );
  }

  /**
   * @brief Setze die neue SPX42 Lizenz
   * @param value neue Lizenz
   */
  void SPX42Config::setLicense( const LicenseType value )
  {
    if ( spxLicense.getLicType() != value )
    {
      spxLicense.setLicType( value );
      if ( sendSignals )
      {
        emit licenseChangedSig( spxLicense );
      }
    }
    // immer setzten
    currentSpxHash = makeSpxHash();
  }

  /**
   * @brief Setze oder lösche Individual-Lizenz
   * @param value
   */
  void SPX42Config::setLicense( const IndividualLicense value )
  {
    if ( spxLicense.getLicInd() != value )
    {
      spxLicense.setLicInd( value );
      if ( sendSignals )
      {
        emit licenseChangedSig( spxLicense );
      }
    }
    // immer setzten
    currentSpxHash = makeSpxHash();
  }

  /**
   * @brief SPX42Config::setLicense
   * @param lic
   * @param ind
   */
  void SPX42Config::setLicense( const QByteArray &lic, const QByteArray &ind )
  {
    // Kommando SPX_LICENSE_STATE
    // komplett: <~45:LS:CE>
    // übergeben LS,CE
    // LS : License State 0=Nitrox,1=Normoxic Trimix,2=Full Trimix
    // CE : Custom Enabled 0= disabled, 1=enabled
    qint8 licVal = static_cast< qint8 >( lic.toInt() );
    qint8 indVal = -1;
    if ( ind != nullptr && !ind.isEmpty() && !ind.isNull() )
    {
      // wurde eine Individuallizenz erwähnt?
      indVal = static_cast< qint8 >( ind.toInt() );
      switch ( indVal )
      {
        default:
        case 0:
          setLicense( IndividualLicense::LIC_NONE );
          break;
        case 1:
          setLicense( IndividualLicense::LIC_INDIVIDUAL );
      }
    }
    //
    // lizenz eintragen
    //
    switch ( licVal )
    {
      default:
      case 0:
        setLicense( LicenseType::LIC_NITROX );
        break;
      case 1:
        setLicense( LicenseType::LIC_NORMOXIX );
        break;
      case 2:
        setLicense( LicenseType::LIC_FULLTMX );
        break;
      case 3:
        setLicense( LicenseType::LIC_MIL );
        break;
    }
    // immer setzten
    currentSpxHash = makeSpxHash();
  }

  /**
   * @brief gib den Lizenznamen als Sting zurück
   * @return
   */
  QString SPX42Config::getLicName() const
  {
    QString individualName = "";
    if ( spxLicense.getLicInd() == IndividualLicense::LIC_INDIVIDUAL )
    {
      individualName = tr( " (I)" );
    }
    switch ( static_cast< int >( spxLicense.getLicType() ) )
    {
      case static_cast< int >( LicenseType::LIC_NITROX ):
        return ( tr( "NITROX" ).append( individualName ) );
      case static_cast< int >( LicenseType::LIC_NORMOXIX ):
        return ( tr( "NORMOXIC TMX" ).append( individualName ) );
      case static_cast< int >( LicenseType::LIC_FULLTMX ):
        return ( tr( "FULL TMX" ).append( individualName ) );
      case static_cast< int >( LicenseType::LIC_MIL ):
        return ( tr( "MILITARY" ).append( individualName ) );
    }
    return ( tr( "UNKNOWN" ) );
  }

  /**
   * @brief Gib ein gesetztes Gas zurück
   * @param num Nummer des Gases (0..7)
   * @return Das Gas
   */
  SPX42Gas &SPX42Config::getGasAt( int num )
  {
    if ( num < 0 || num > GASES )
    {
      return ( gasList[ 0 ] );
    }
    return ( gasList[ num ] );
  }

  void SPX42Config::setGasAt( int num, const SPX42Gas &gas )
  {
    if ( num < 0 || num > 7 )
    {
      return;
    }
    //
    // Gas setzten und hash berechnen
    //
    gasList[ num ] = gas;
    QByteArray serialized = gasList[ num ].serialize();
    qhash.reset();
    qhash.addData( serialized );
    currentGasHashes[ num ] = qhash.result();
  }

  /**
   * @brief Setze das Objekt auf einen definierten Grundzustand
   */
  void SPX42Config::resetConfig( quint8 classes )
  {
    isValid = false;
    if ( classes & SPX42ConfigClass::CF_CLASS_SPX )
    {
      spxLicense.setLicType( LicenseType::LIC_NITROX );
      spxLicense.setLicInd( IndividualLicense::LIC_NONE );
      spxSerialNumber = "0000000000";
      hasFahrenheidBug = false;
      canSetDate = false;
      hasSixValuesIndividual = true;
      isFirmwareSupported = false;
      isOldParamSorting = false;
      isNewerDisplayBrightness = true;
      isSixMetersAutoSetpoint = true;
      currentSpxHash.clear();
      savedSpxHash.clear();
    }
    if ( classes & SPX42ConfigClass::CF_CLASS_GASES )
    {
      for ( int i = 0; i < GAS_HASHES; i++ )
      {
        gasList[ i ].reset();
        currentGasHashes[ i ].clear();
        savedGasHashes[ i ].clear();
      }
    }
    if ( classes & SPX42ConfigClass::CF_CLASS_DECO )
    {
      decoCurrentPreset = DecompressionPreset::DECO_KEY_CONSERVATIVE;
      decoDeepstopsEnabled = DecompressionDeepstops::DEEPSTOPS_ENABLED;
      decoDynamicGradient = DecompressionDynamicGradient::DYNAMIC_GRADIENT_ON;
      currentDecoHash.clear();
      savedDecoHash.clear();
    }
    if ( classes & SPX42ConfigClass::CF_CLASS_DISPLAY )
    {
      displayBrightness = DisplayBrightness::BRIGHT_100;
      displayOrientation = DisplayOrientation::LANDSCAPE;
      currentDisplayHash.clear();
      savedDisplayHash.clear();
    }
    if ( classes & SPX42ConfigClass::CF_CLASS_UNITS )
    {
      unitTemperature = DeviceTemperaturUnit::CELSIUS;
      unitLength = DeviceLenghtUnit::METRIC;
      unitWaterType = DeviceWaterType::FRESHWATER;
      currentUnitHash.clear();
      savedUnitHash.clear();
    }
    if ( classes & SPX42ConfigClass::CF_CLASS_SETPOINT )
    {
      setpointAuto = DeviceSetpointAuto::AUTO_06;
      setpointValue = DeviceSetpointValue::SETPOINT_10;
      currentSetpointHash.clear();
      savedSetpointHash.clear();
    }
    if ( classes & SPX42ConfigClass::CF_CLASS_INDIVIDUAL )
    {
      individualSensorsOn = DeviceIndividualSensors::SENSORS_ON;
      individualPSCROn = DeviceIndividualPSCR::PSCR_OFF;
      individualSensorCount = DeviceIndividualSensorCount::SENSOR_COUNT_03;
      individualAcustic = DeviceIndividualAcoustic::ACOUSTIC_ON;
      individualLogInterval = DeviceIndividualLogInterval::INTERVAL_20;
      individualTempStick = DeviceIndividualTempstick::TEMPSTICK01;
      currentIndividualHash.clear();
      savedIndividualHash.clear();
    }
    emit licenseChangedSig( spxLicense );
  }

  /**
   * @brief Eefrage die gespeicherte SPX42 Seriennummer
   * @return
   */
  QString SPX42Config::getSerialNumber() const
  {
    if ( spxSerialNumber.isEmpty() || spxSerialNumber.isNull() )
      return ( "000000" );
    return ( spxSerialNumber );
  }

  /**
   * @brief Setzte die Seriennummer für den SPX42
   * @param serial Seriennummer als String
   */
  void SPX42Config::setSerialNumber( const QString &serial )
  {
    if ( spxSerialNumber != serial )
    {
      spxSerialNumber = serial;
      if ( sendSignals )
      {
        emit serialNumberChangedSig( spxSerialNumber );
      }
    }
    // immer setzten
    currentSpxHash = makeSpxHash();
  }

  /**
   * @brief Setzte die Dekompressions-Voreinstellung
   * @param presetType der Typ des Presets
   * @param low LOW Gradient, wenn Preset == CUSTOM
   * @param high HIGH Gradient, wenn Preset == CUSTOM
   */
  void SPX42Config::setCurrentPreset( DecompressionPreset presetType, qint8 low, qint8 high )
  {
    setCurrentPreset( presetType, DecoGradient( low, high ) );
  }

  /**
   * @brief Setzte die Dekompressions-Voreinstellung
   * @param presetType der Typ des Presets
   * @param dGraient Einstellung für Deko-Gradient
   */
  void SPX42Config::setCurrentPreset( DecompressionPreset presetType, const DecoGradient &dGradient )
  {
    //
    // wenn CUSTOM gegeben ist, dann die Werte eintragen
    //
    if ( presetType == DecompressionPreset::DECO_KEY_CUSTOM )
    {
      //
      // insert macht ein update....
      //
      decoPresets.insert( static_cast< int >( DecompressionPreset::DECO_KEY_CUSTOM ),
                          DecoGradient( dGradient.first, dGradient.second ) );
      decoCurrentPreset = presetType;
      if ( sendSignals )
      {
        emit decoGradientChangedSig( getCurrentDecoGradientValue() );
      }
    }
    else
    {
      //
      // ein PRESET ausgewählt werte verwerfen...
      //
      decoCurrentPreset = presetType;
      if ( sendSignals )
      {
        emit decoGradientChangedSig( getCurrentDecoGradientValue() );
      }
    }
    currentDecoHash = makeDecoHash();
  }

  /**
   * @brief Gib das aktuelle Preset zurück
   * @return
   */
  DecompressionPreset SPX42Config::getCurrentDecoGradientPresetType()
  {
    return ( decoCurrentPreset );
  }

  /**
   * @brief Gib die Gradienten zurück
   * @return Die Gradienten als Objekt DecoGradient
   */
  DecoGradient SPX42Config::getCurrentDecoGradientValue() const
  {
    return ( decoPresets.value( static_cast< int >( decoCurrentPreset ) ) );
  }

  /**
   * @brief Gib die Grdienten aür einen bestimmten Typ zurück
   * @param presetType der Tp, für den die Gradienten gesucht werden
   * @return Die Gradienten als Objekt DecoGradient
   */
  DecoGradient SPX42Config::getPresetValues( DecompressionPreset presetType ) const
  {
    //
    // gib einfach das Wertepaar zurück
    //
    return ( decoPresets.value( static_cast< int >( presetType ) ) );
  }

  /**
   * @brief Ermittle das Preset für angegebene Werte
   * @param Gradient low
   * @param Gradient high
   * @return Preset-Typ
   */
  DecompressionPreset SPX42Config::getPresetForGradient( qint8 low, qint8 high )
  {
    for ( DecoGradientHash::iterator it = decoPresets.begin(); it != decoPresets.end(); it++ )
    {
      if ( it.value().first == low && it.value().second == high )
      {
        return ( static_cast< DecompressionPreset >( it.key() ) );
      }
    }
    return ( DecompressionPreset::DECO_KEY_CUSTOM );
  }

  /**
   * @brief gib aktuelle dynamische Gradienten zurück
   * @return Gradientenobjekt
   */
  DecompressionDynamicGradient SPX42Config::getIsDecoDynamicGradients()
  {
    return ( decoDynamicGradient );
  }

  /**
   * @brief setzte dynamische Gadienten an/aus
   * @param isDynamicEnabled
   */
  void SPX42Config::setIsDecoDynamicGradients( DecompressionDynamicGradient isDynamicEnabled )
  {
    if ( decoDynamicGradient != isDynamicEnabled )
    {
      decoDynamicGradient = isDynamicEnabled;
      if ( sendSignals )
      {
        emit decoDynamicGradientStateChangedSig( decoDynamicGradient );
      }
    }
    currentDecoHash = makeDecoHash();
  }

  /**
   * @brief erfrage ob tiefe Dekostops erlaubt sind
   * @return sind Dekostops erlaubt?
   */
  DecompressionDeepstops SPX42Config::getIstDeepstopsEnabled()
  {
    return ( decoDeepstopsEnabled );
  }

  /**
   * @brief SPX42Config::setIsDeepstopsEnabled
   * @param isEnabled
   */
  void SPX42Config::setIsDeepstopsEnabled( DecompressionDeepstops isEnabled )
  {
    if ( decoDeepstopsEnabled != isEnabled )
    {
      decoDeepstopsEnabled = isEnabled;
      if ( sendSignals )
      {
        emit decoDeepStopsEnabledSig( decoDeepstopsEnabled );
      }
    }
    currentDecoHash = makeDecoHash();
  }

  /**
   * @brief SPX42Config::getLastDecoStop
   * @return
   */
  DecoLastStop SPX42Config::getLastDecoStop( void )
  {
    return ( decoLastStop );
  }

  /**
   * @brief SPX42Config::setLastDecoStop
   * @param lastStop
   */
  void SPX42Config::setLastDecoStop( DecoLastStop lastStop )
  {
    if ( lastStop != decoLastStop )
    {
      decoLastStop = lastStop;
      if ( sendSignals )
      {
        emit decoLastStopSig( decoLastStop );
      }
    }
    currentDecoHash = makeDecoHash();
  }

  /**
   * @brief Gib die einsgestellte Displayhelligkeit zurück
   * @return Helligkeit als Objekt
   */
  DisplayBrightness SPX42Config::getDisplayBrightness()
  {
    return ( displayBrightness );
  }

  /**
   * @brief Setzte Display Helligkeit
   * @param brightness die Helligkeit
   */
  void SPX42Config::setDisplayBrightness( DisplayBrightness brightness )
  {
    if ( displayBrightness != brightness )
    {
      displayBrightness = brightness;
      if ( sendSignals )
      {
        emit displayBrightnessChangedSig( displayBrightness );
      }
    }
    currentDisplayHash = makeDisplayHash();
  }

  /**
   * @brief Gib die Helligkeit als Objekt zurück
   * @return Helligkeit als Objekt
   */
  DisplayOrientation SPX42Config::getDisplayOrientation()
  {
    return ( displayOrientation );
  }

  /**
   * @brief SPX42Config::setDisplayOrientation
   * @param orientation
   */
  void SPX42Config::setDisplayOrientation( DisplayOrientation orientation )
  {
    if ( displayOrientation != orientation )
    {
      displayOrientation = orientation;
      if ( sendSignals )
      {
        emit displayOrientationChangedSig( displayOrientation );
      }
    }
    currentDisplayHash = makeDisplayHash();
  }

  /**
   * @brief gib die Temperatureinheit als Objekt zurück
   * @return Temperatureinheit als Objekt
   */
  DeviceTemperaturUnit SPX42Config::getUnitsTemperatur()
  {
    return ( unitTemperature );
  }

  /**
   * @brief Setzte die Temperatureiheit
   * @param tUnit Wert der Einheit
   */
  void SPX42Config::setUnitsTemperatur( DeviceTemperaturUnit tUnit )
  {
    if ( unitTemperature != tUnit )
    {
      unitTemperature = tUnit;
      if ( sendSignals )
      {
        emit unitsTemperaturChangedSig( unitTemperature );
      }
    }
    currentUnitHash = makeUnitsHash();
  }

  /**
   * @brief Gib die Einheit der Länge zurück
   * @return Einheit der Länge als Objekt
   */
  DeviceLenghtUnit SPX42Config::getUnitsLength()
  {
    return ( unitLength );
  }

  /**
   * @brief Setzte die Einheit der Längen
   * @param lUnit Einhait als Objekt
   */
  void SPX42Config::setUnitsLength( DeviceLenghtUnit lUnit )
  {
    if ( unitLength != lUnit )
    {
      unitLength = lUnit;
      if ( sendSignals )
      {
        emit unitsLengtChangedSig( unitLength );
      }
    }
    currentUnitHash = makeUnitsHash();
  }

  /**
   * @brief Gib den Wassertyp zurück
   * @return Wassertyp als Objekt
   */
  DeviceWaterType SPX42Config::getUnitsWaterType()
  {
    return ( unitWaterType );
  }

  /**
   * @brief setzte den Wassertyp
   * @param wUnit Wassertyp als Objekt
   */
  void SPX42Config::setUnitsWaterType( DeviceWaterType wUnit )
  {
    if ( unitWaterType != wUnit )
    {
      unitWaterType = wUnit;
      if ( sendSignals )
      {
        emit untisWaterTypeChangedSig( unitWaterType );
      }
    }
    currentUnitHash = makeUnitsHash();
  }

  /**
   * @brief Gib den Autosetpoint zurück
   * @return Autosetpoint als Obhekt
   */
  DeviceSetpointAuto SPX42Config::getSetpointAuto()
  {
    return ( setpointAuto );
  }

  /**
   * @brief setzte den Autosetpoint
   * @param aSetpoint autosetpoint als Objekt
   */
  void SPX42Config::setSetpointAuto( DeviceSetpointAuto aSetpoint )
  {
    if ( setpointAuto != aSetpoint )
    {
      setpointAuto = aSetpoint;
      if ( sendSignals )
      {
        emit setpointAutoChangeSig( setpointAuto );
      }
    }
    // mach hier den hash immer
    currentSetpointHash = makeSetpointHash();
  }

  /**
   * @brief Gib den Setpoint zurück
   * @return der Setpoint als Obhekt
   */
  DeviceSetpointValue SPX42Config::getSetpointValue()
  {
    return ( setpointValue );
  }

  /**
   * @brief setzte den Setpoint
   * @param ppo2 als Objekt
   */
  void SPX42Config::setSetpointValue( DeviceSetpointValue ppo2 )
  {
    if ( setpointValue != ppo2 )
    {
      setpointValue = ppo2;
      if ( sendSignals )
      {
        emit setpointValueChangedSig( setpointValue );
      }
    }
    // mache den Hash immer
    currentSetpointHash = makeSetpointHash();
  }

  /**
   * @brief gib die Einstellung Sensoren ein/aus zurück
   * @return Einstellung der Sensoren (ein/aus)
   */
  DeviceIndividualSensors SPX42Config::getIndividualSensorsOn()
  {
    return ( individualSensorsOn );
  }

  /**
   * @brief setzte die Einstellunge Sensoren (ein/aus)
   * @param onOff Wert der Einstellung
   */
  void SPX42Config::setIndividualSensorsOn( DeviceIndividualSensors onOff )
  {
    if ( individualSensorsOn != onOff )
    {
      individualSensorsOn = onOff;
      if ( sendSignals )
      {
        emit individualSensorsOnChangedSig( individualSensorsOn );
      }
    }
    currentIndividualHash = makeIndividualHash();
  }

  /**
   * @brief SPX42Config::getIndividualPscrMode
   * @return Wert der Einstellung als Objekt
   */
  DeviceIndividualPSCR SPX42Config::getIndividualPscrMode()
  {
    return ( individualPSCROn );
  }

  /**
   * @brief Setzte die Einstellung für Passive Semiclosed Rebreather (ein/aus)
   * @param pscrMode Einstellung als Objekt
   */
  void SPX42Config::setIndividualPscrMode( DeviceIndividualPSCR pscrMode )
  {
    if ( individualPSCROn != pscrMode )
    {
      if ( sendSignals )
      {
        emit individualPscrModeChangedSig( individualPSCROn );
      }
    }
    currentIndividualHash = makeIndividualHash();
  }

  /**
   * @brief gib die Anzahl der Sensoren zurück
   * @return Anzahl der Sensoren als Objekt
   */
  DeviceIndividualSensorCount SPX42Config::getIndividualSensorsCount()
  {
    return ( individualSensorCount );
  }

  /**
   * @brief Setzte die Anzahl der aktiven Sensoren
   * @param sCount Anzahl als Objekt
   */
  void SPX42Config::setIndividualSensorsCount( DeviceIndividualSensorCount sCount )
  {
    if ( individualSensorCount != sCount )
    {
      if ( sendSignals )
      {
        emit individualSensorsCountChangedSig( individualSensorCount );
      }
    }
    currentIndividualHash = makeIndividualHash();
  }

  /**
   * @brief Gib die Einstellung für akustische Warnungen zurück
   * @return Einstellung für akustische Warnungen als Objekt
   */
  DeviceIndividualAcoustic SPX42Config::getIndividualAcoustic()
  {
    return ( individualAcustic );
  }

  /**
   * @brief Setze Einstellung zr akustichen Warnung (ein/aus)
   * @param acoustic Einstellung als Objekt
   */
  void SPX42Config::setIndividualAcoustic( DeviceIndividualAcoustic acoustic )
  {
    if ( individualAcustic != acoustic )
    {
      individualAcustic = acoustic;
      if ( sendSignals )
      {
        emit individualAcousticChangedSig( individualAcustic );
      }
    }
    currentIndividualHash = makeIndividualHash();
  }

  /**
   * @brief gib die Einstellung für das Loginterval zurück
   * @return Einstellung als Objekt
   */
  DeviceIndividualLogInterval SPX42Config::getIndividualLogInterval()
  {
    return ( individualLogInterval );
  }

  /**
   * @brief Setzte Einstellung für das Loginterval
   * @param logInterval Interval als Objjekt
   */
  void SPX42Config::setIndividualLogInterval( DeviceIndividualLogInterval logInterval )
  {
    if ( individualLogInterval != logInterval )
    {
      individualLogInterval = logInterval;
      if ( sendSignals )
      {
        emit individualLogIntervalChangedSig( individualLogInterval );
      }
    }
    currentIndividualHash = makeIndividualHash();
  }

  /**
   * @brief Erfrage die Einstellungen für den Tempstick
   * @return Einstellung des Tempsticks
   */
  DeviceIndividualTempstick SPX42Config::getIndividualTempStick()
  {
    return ( individualTempStick );
  }

  /**
   * @brief Setze Einstellung für Typ des Tempertursticks
   * @param tStick Einstellung als Objekt
   */
  void SPX42Config::setIndividualTempStick( DeviceIndividualTempstick tStick )
  {
    if ( individualTempStick != tStick )
    {
      individualTempStick = tStick;
      if ( sendSignals )
      {
        emit individualTempstickChangedSig( individualTempStick );
      }
    }
    currentIndividualHash = makeIndividualHash();
  }

  /**
   * @brief SPX42Config::getHasFahrenheidBug
   * @return
   */
  bool SPX42Config::getHasFahrenheidBug() const
  {
    return hasFahrenheidBug;
  }

  /**
   * @brief SPX42Config::getCanSetDate
   * @return
   */
  bool SPX42Config::getCanSetDate() const
  {
    return canSetDate;
  }

  /**
   * @brief SPX42Config::getHasSixValuesIndividual
   * @return
   */
  bool SPX42Config::getHasSixValuesIndividual() const
  {
    return hasSixValuesIndividual;
  }

  /**
   * @brief SPX42Config::getIsFirmwareSupported
   * @return
   */
  bool SPX42Config::getIsFirmwareSupported() const
  {
    return isFirmwareSupported;
  }

  /**
   * @brief SPX42Config::getIsOldParamSorting
   * @return
   */
  bool SPX42Config::getIsOldParamSorting() const
  {
    return isOldParamSorting;
  }

  /**
   * @brief SPX42Config::getIsNewerDisplayBrightness
   * @return
   */
  bool SPX42Config::getIsNewerDisplayBrightness() const
  {
    return isNewerDisplayBrightness;
  }

  /**
   * @brief SPX42Config::getIsSixMetersAutoSetpoint
   * @return
   */
  bool SPX42Config::getIsSixMetersAutoSetpoint() const
  {
    return isSixMetersAutoSetpoint;
  }

  /**
   * @brief SPX42Config::makeSpxHash
   * @return
   */
  QString SPX42Config::makeSpxHash()
  {
    QByteArray serialized;
    serialized.append( QString( "firmware version (enum): %1\n" ).arg( static_cast< int >( spxFirmwareVersion ) ) );
    serialized.append( spxLicense.serialize() );
    serialized.append( QString( "serial number: %1" ).arg( spxSerialNumber ) );
    qhash.reset();
    qhash.addData( serialized );
    return ( QString( qhash.result().toBase64() ) );
  }

  /**
   * @brief SPX42Config::makeDecoHash
   * @return
   */
  QString SPX42Config::makeDecoHash()
  {
    QByteArray serialized;
    serialized.append( QString( "deco preset (enum): %1" ).arg( static_cast< int >( decoCurrentPreset ) ) );
    serialized.append(
        QString( "low gradient: %1" ).arg( decoPresets.value( static_cast< int >( decoCurrentPreset ) ).first, 2, 10, QChar( '0' ) ) );
    serialized.append( QString( "high gradient: %1" )
                           .arg( decoPresets.value( static_cast< int >( decoCurrentPreset ) ).second, 2, 10, QChar( '0' ) ) );
    serialized.append( QString( "dynamic gradients: %1" ).arg( static_cast< bool >( decoDynamicGradient ) ) );
    serialized.append( QString( "deep stops enabled: %1" ).arg( static_cast< bool >( decoDeepstopsEnabled ) ) );
    serialized.append( QString( "last deco stop (enum): %1" ).arg( static_cast< int >( decoLastStop ) ) );
    qhash.reset();
    qhash.addData( serialized );
    return ( QString( qhash.result().toBase64() ) );
  }

  /**
   * @brief SPX42Config::makeDisplayHash
   * @return
   */
  QString SPX42Config::makeDisplayHash()
  {
    QByteArray serialized;
    serialized.append(
        QString( "display brightness (enum): %1" ).arg( static_cast< int >( displayBrightness ), 2, 10, QChar( '0' ) ) );
    serialized.append(
        QString( "display orientation (enum): %1" ).arg( static_cast< int >( displayOrientation ), 2, 10, QChar( '0' ) ) );
    qhash.reset();
    qhash.addData( serialized );
    return ( QString( qhash.result().toBase64() ) );
  }

  /**
   * @brief SPX42Config::makeUnitsHash
   * @return
   */
  QString SPX42Config::makeUnitsHash()
  {
    QByteArray serialized;
    serialized.append(
        QString( "unit for temperature (enum): %1" ).arg( static_cast< int >( unitTemperature ), 2, 10, QChar( '0' ) ) );
    serialized.append( QString( "unit for lenght (enum): %1" ).arg( static_cast< int >( unitLength ), 2, 10, QChar( '0' ) ) );
    serialized.append( QString( "water type (enum): %1" ).arg( static_cast< int >( unitWaterType ), 2, 10, QChar( '0' ) ) );
    qhash.reset();
    qhash.addData( serialized );
    return ( QString( qhash.result().toBase64() ) );
  }

  /**
   * @brief SPX42Config::makeSetpointHash
   * @return
   */
  QString SPX42Config::makeSetpointHash()
  {
    QByteArray serialized;
    serialized.append( QString( "auto setpoint value (enum): %1" ).arg( static_cast< int >( setpointAuto ), 2, 10, QChar( '0' ) ) );
    serialized.append( QString( "setpoint value (enum): %1" ).arg( static_cast< int >( setpointValue ), 2, 10, QChar( '0' ) ) );
    qhash.reset();
    qhash.addData( serialized );
    return ( QString( qhash.result().toBase64() ) );
  }

  /**
   * @brief SPX42Config::makeIndividualHash
   * @return
   */
  QString SPX42Config::makeIndividualHash()
  {
    QByteArray serialized;
    serialized.append( QString( "individual sensors on: %1" ).arg( static_cast< bool >( individualSensorsOn ) ) );
    serialized.append( QString( "indiovidual pscr on: %1" ).arg( static_cast< bool >( individualPSCROn ) ) );
    serialized.append(
        QString( "individual sensors count: %1" ).arg( static_cast< int >( individualSensorCount ), 2, 10, QChar( '0' ) ) + 1 );
    serialized.append( QString( "individual acoustic warnings on: %1" ).arg( static_cast< bool >( individualAcustic ) ) );
    serialized.append( QString( "individual log interval: %1" )
                           .arg( ( static_cast< int >( individualLogInterval ) + 1 ) * 10, 2, 10, QChar( '0' ) ) );
    serialized.append(
        QString( "individual temp stick type (enum): %1" ).arg( static_cast< int >( individualTempStick ), 2, 10, QChar( '0' ) ) );
    qhash.reset();
    qhash.addData( serialized );
    return ( QString( qhash.result().toBase64() ) );
  }

  /**
   * @brief SPX42Config::freezeConfigs
   */
  void SPX42Config::freezeConfigs( quint8 changed )
  {
    //
    // was soll gefrostet werden?
    //
    if ( changed & SPX42ConfigClass::CF_CLASS_SPX )
      savedSpxHash = currentSpxHash;
    if ( changed & SPX42ConfigClass::CF_CLASS_DECO )
      savedDecoHash = currentDecoHash;
    if ( changed & SPX42ConfigClass::CF_CLASS_GASES )
    {
      for ( int i = 0; i < GAS_HASHES; i++ )
      {
        savedGasHashes[ i ] = currentGasHashes[ i ];
      }
    }
    if ( changed & SPX42ConfigClass::CF_CLASS_DISPLAY )
      savedDisplayHash = currentDisplayHash;
    if ( changed & SPX42ConfigClass::CF_CLASS_UNITS )
      savedUnitHash = currentUnitHash;
    if ( changed & SPX42ConfigClass::CF_CLASS_SETPOINT )
      savedSetpointHash = currentSetpointHash;
    if ( changed & SPX42ConfigClass::CF_CLASS_INDIVIDUAL )
      savedIndividualHash = currentIndividualHash;
  }

  /**
   * @brief SPX42Config::getChangedConfig
   * @return
   */
  quint8 SPX42Config::getChangedConfig( void )
  {
    quint8 result = 0;
    if ( currentSpxHash != savedSpxHash )
      result |= SPX42ConfigClass::CF_CLASS_SPX;
    if ( currentDecoHash != savedDecoHash )
      result |= SPX42ConfigClass::CF_CLASS_DECO;
    if ( currentDisplayHash != savedDisplayHash )
      result |= SPX42ConfigClass::CF_CLASS_DISPLAY;
    if ( currentSetpointHash != savedSetpointHash )
      result |= SPX42ConfigClass::CF_CLASS_SETPOINT;
    if ( currentUnitHash != savedUnitHash )
      result |= SPX42ConfigClass::CF_CLASS_UNITS;
    if ( currentIndividualHash != savedIndividualHash )
      result |= SPX42ConfigClass::CF_CLASS_INDIVIDUAL;
    for ( int i = 0; i < GAS_HASHES; i++ )
    {
      if ( savedGasHashes[ i ] != currentGasHashes[ i ] )
      {
        result |= SPX42ConfigClass::CF_CLASS_GASES;
        // schleife verlassen
        break;
      }
    }
    return ( result );
  }

  quint8 SPX42Config::getChangedGases()
  {
    quint8 result = 0;
    if ( savedGasHashes[ 0 ] != currentGasHashes[ 0 ] )
      result |= SPX42ConfigClass::CF_GAS01;
    if ( savedGasHashes[ 1 ] != currentGasHashes[ 1 ] )
      result |= SPX42ConfigClass::CF_GAS02;
    if ( savedGasHashes[ 2 ] != currentGasHashes[ 2 ] )
      result |= SPX42ConfigClass::CF_GAS03;
    if ( savedGasHashes[ 3 ] != currentGasHashes[ 3 ] )
      result |= SPX42ConfigClass::CF_GAS04;
    if ( savedGasHashes[ 4 ] != currentGasHashes[ 4 ] )
      result |= SPX42ConfigClass::CF_GAS05;
    if ( savedGasHashes[ 5 ] != currentGasHashes[ 5 ] )
      result |= SPX42ConfigClass::CF_GAS06;
    if ( savedGasHashes[ 6 ] != currentGasHashes[ 6 ] )
      result |= SPX42ConfigClass::CF_GAS07;
    if ( savedGasHashes[ 7 ] != currentGasHashes[ 7 ] )
      result |= SPX42ConfigClass::CF_GAS08;
    return ( result );
  }

  QString SPX42Config::geteUnitHashes( void )
  {
    return ( QString( "unit current hash: %1, unit saved hash: %2" ).arg( currentUnitHash ).arg( savedUnitHash ) );
  }
}  // namespace spx
