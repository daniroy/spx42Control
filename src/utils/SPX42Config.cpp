﻿#include "SPX42Config.hpp"

namespace spx42
{
  /**
   * @brief Standartkonstruktor
   */
  SPX42Config::SPX42Config() :
    QObject(Q_NULLPTR),
    sendSignals( true )
  {
    reset();
    connect( &spxLicense, &SPX42License::licenseChangedPrivateSig, this, &SPX42Config::licenseChangedPrivateSlot );
    //
    // DECO Presets füllen
    //
    decoPresets.insert( static_cast<int>(DecompressionPreset::DECO_KEY_V_CONSERVATIVE), ProjectConst::DECO_VAL_V_CONSERVATIVE );
    decoPresets.insert( static_cast<int>(DecompressionPreset::DECO_KEY_CONSERVATIVE), ProjectConst::DECO_VAL_CONSERVATIVE );
    decoPresets.insert( static_cast<int>(DecompressionPreset::DECO_KEY_MODERATE), ProjectConst::DECO_VAL_MODERATE );
    decoPresets.insert( static_cast<int>(DecompressionPreset::DECO_KEY_AGRESSIVE), ProjectConst::DECO_VAL_AGRESSIVE );
    decoPresets.insert( static_cast<int>(DecompressionPreset::DECO_KEY_V_AGRESSIVE), ProjectConst::DECO_VAL_V_AGRESSIVE );
    decoPresets.insert( static_cast<int>(DecompressionPreset::DECO_KEY_CUSTOM), ProjectConst::DECO_VAL_CUSTOM );
  }

  SPX42License& SPX42Config::getLicense()
  {
    return spxLicense;
  }

  void SPX42Config::setLicense(const SPX42License value)
  {
    if( spxLicense != value )
    {
      spxLicense.setLicType( value.getLicType());
      if( sendSignals )
      {
        emit licenseChangedSig( spxLicense );
      }
    }
  }


  QString SPX42Config::getLicName() const
  {
    switch( static_cast<int>(spxLicense.getLicType()))
    {
      case static_cast<int>(LicenseType::LIC_NITROX):
        return( tr("NITROX") );
      case static_cast<int>(LicenseType::LIC_NORMOXIX):
        return( tr("NORMOXIC TMX") );
      case static_cast<int>(LicenseType::LIC_FULLTMX):
        return( tr("FULL TMX") );
      case static_cast<int>(LicenseType::LIC_MIL):
        return( tr("MILITARY") );
    }
    return( tr("UNKNOWN") );
  }

  SPX42Gas& SPX42Config::getGasAt( int num )
  {
    if( num < 0 || num > 7 )
    {
      return( gasList[0] );
    }
    return( gasList[num]);
  }

  void SPX42Config::reset(void)
  {
    isValid = false;
    spxLicense.setLicType( LicenseType::LIC_NITROX );
    spxLicense.setLicInd( IndividualLicense::LIC_NONE );
    serialNumber = "0000000000";
    for( int i=0; i<8; i++)
    {
      gasList[0].reset();
    }
    currentPreset = DecompressionPreset::DECO_KEY_CONSERVATIVE;
    decoDeepstopsEnabled = true;
    decoDynamicGradient = true;
    displayBrightness = DisplayBrightness::BRIGHT_100;
    displayOrientation = DisplayOrientation::LANDSCAPE;
    emit licenseChangedSig( spxLicense );
  }

  QString SPX42Config::getSerialNumber() const
  {
    return( serialNumber );
  }

  void SPX42Config::setSerialNumber(const QString &serial)
  {
    if( serialNumber != serial )
    {
      serialNumber = serial;
      if( sendSignals  )
      {
        emit serialNumberChangedSig( serialNumber );
      }
    }
  }

  void SPX42Config::setCurrentPreset( DecompressionPreset presetType, qint8 low, qint8 high )
  {
    //
    // wenn CUSTOM gegeben ist, dann die Werte eintragen
    //
    if( presetType == DecompressionPreset::DECO_KEY_CUSTOM )
    {
      //
      // insert macht ein update....
      //
      decoPresets.insert( static_cast<int>(DecompressionPreset::DECO_KEY_CUSTOM), DecoGradient(low,high) );
      if( sendSignals )
      {
        emit decoGradientChangedSig( getCurrentDecoGradientValue() );
      }
      return;
    }
    //
    // oder wenn sich der Typ geändert hat
    //
    if( currentPreset != presetType )
    {
      currentPreset = presetType;
      if( sendSignals )
      {
        emit decoGradientChangedSig( getCurrentDecoGradientValue() );
      }
    }
    // oder nix geändert, nix machen
  }

  DecompressionPreset SPX42Config::getCurrentDecoGradientPresetType()
  {
    return( currentPreset );
  }

  DecoGradient SPX42Config::getCurrentDecoGradientValue()
  {
    return( decoPresets.value( static_cast<int>(currentPreset) ));
  }

  DecoGradient SPX42Config::getPresetValues( DecompressionPreset presetType ) const
  {
    //
    // gib einfach das Wertepaar zurück
    //
    return( decoPresets.value( static_cast<int>(presetType) ) );
  }

  /**
   * @brief Ermittle das Preset für angegebene Werte
   * @param Gradient low
   * @param Gradient high
   * @return Preset-Typ
   */
  DecompressionPreset SPX42Config::getPresetForGradient( qint8 low, qint8 high )
  {
    for( DecoGradientHash::iterator it = decoPresets.begin(); it != decoPresets.end(); it++ )
    {
      if( it.value().first == low && it.value().second == high )
      {
        return( static_cast<DecompressionPreset>(it.key()));
      }
    }
    return( DecompressionPreset::DECO_KEY_CUSTOM );
  }


  void SPX42Config::licenseChangedPrivateSlot(void)
  {
    if( sendSignals )
    {
      emit licenseChangedSig( spxLicense );
    }
  }

  bool SPX42Config::getIsDecoDynamicGradients(void)
  {
    return( decoDynamicGradient );
  }

  void SPX42Config::setIsDecoDynamicGradients( bool isDynamicEnabled )
  {
    if( decoDynamicGradient != isDynamicEnabled )
    {
      decoDynamicGradient = isDynamicEnabled;
      if( sendSignals )
      {
        emit decoDynamicGradientStateChangedSig( decoDynamicGradient );
      }
    }
  }

  bool SPX42Config::getIstDeepstopsEnabled( void )
  {
    return( decoDeepstopsEnabled );
  }

  void SPX42Config::setIsDeepstopsEnabled( bool isEnabled )
  {
    if( decoDeepstopsEnabled != isEnabled )
    {
      decoDeepstopsEnabled = isEnabled;
      if( sendSignals )
      {
        emit decoDeepStopsEnabledSig( decoDeepstopsEnabled );
      }
    }
  }

  DisplayBrightness SPX42Config::getDisplayBrightness( void )
  {
    return( displayBrightness );
  }

  void SPX42Config::setDisplayBrightness( DisplayBrightness brightness )
  {
    if( displayBrightness != brightness )
    {
      displayBrightness = brightness;
      if( sendSignals )
      {
        emit displayBrightnessChangedSig( displayBrightness );
      }
    }
  }

  DisplayOrientation SPX42Config::getDisplayOrientation( void )
  {
    return( displayOrientation );
  }

  void SPX42Config::setDisplayOrientation( DisplayOrientation orientation )
  {
    if( displayOrientation != orientation )
    {
      displayOrientation = orientation;
      if( sendSignals )
      {
        emit displayOrientationChangedSig( displayOrientation );
      }
    }
  }

}
