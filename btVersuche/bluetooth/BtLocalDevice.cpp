#include "BtLocalDevice.hpp"

/**
 * @brief BtLocalDevice::BtLocalDevice
 */
BtLocalDevice::BtLocalDevice( std::shared_ptr< Logger > logger,
                              QObject *parent )
    : QObject( parent ), lg( logger ), localDevice( new QBluetoothLocalDevice )
{
  //
  // neuen Device Discovering Agenten machen
  //
  lg->debug( "BtLocalDevice::BtLocalDevice: create device discovery agent..." );
  discoveryAgent = new QBluetoothDeviceDiscoveryAgent();
  //
  // Signale mit Slots verbinden
  //
  lg->debug( "BtLocalDevice::BtLocalDevice: connect signals..." );
  connect( discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
           this, &BtLocalDevice::addDevice );
  connect( discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this,
           &BtLocalDevice::scanFinished );
  connect( localDevice, &QBluetoothLocalDevice::hostModeStateChanged, this,
           &BtLocalDevice::hostModeStateChanged );
  connect( localDevice, &QBluetoothLocalDevice::pairingFinished, this,
           &BtLocalDevice::pairingDone );
  //
  // den richtigen Hostmode einstellen (via slot)
  //
  hostModeStateChanged( localDevice->hostMode() );
}

/**
 * @brief BtLocalDevice::~BtLocalDevice
 */
BtLocalDevice::~BtLocalDevice()
{
  lg->debug( "BtLocalDevice::~BtLocalDevice..." );
  delete discoveryAgent;
}

void BtLocalDevice::startDeviceScan( void )
{
  lg->info( "BtLocalDevice::startDeviceScan: start device scanning..." );
  discoveryAgent->start();
  emit sigDiscoverStarted();
}

void BtLocalDevice::scanFinished( void )
{
  lg->info(
      "BtLocalDevice::startDeviceScan: start device scanning finished..." );
  emit sigDiscoverFinished();
}

void BtLocalDevice::addDevice( const QBluetoothDeviceInfo &info )
{
  bool paired;  // ist device gepaart
  // pairing status abfragen
  QBluetoothLocalDevice::Pairing pairingStatus =
      localDevice->pairingStatus( info.address() );
  if ( pairingStatus == QBluetoothLocalDevice::Paired ||
       pairingStatus == QBluetoothLocalDevice::AuthorizedPaired )
  {
    paired = true;
  }
  else
  {
    paired = false;
  }
  lg->debug( QString( "BtLocalDevice::addDevice: %1, %2 gepaart: %3 ..." )
                 .arg( info.address().toString() )
                 .arg( info.name() )
                 .arg( paired ) );
  emit sigFoundDevice( info );
}

void BtLocalDevice::hostModeStateChanged( QBluetoothLocalDevice::HostMode mode )
{
  switch ( mode )
  {
    case QBluetoothLocalDevice::HostPoweredOff:
      lg->debug( "host powered off" );
      break;
    case QBluetoothLocalDevice::HostConnectable:
      lg->debug( "host is connectable" );
      break;
    case QBluetoothLocalDevice::HostDiscoverable:
      lg->debug( "host is discoverable" );
      break;
    case QBluetoothLocalDevice::HostDiscoverableLimitedInquiry:
      lg->debug( "host is limited inquiryable " );
  }
  // TODO: Signal über Änderung senden?
}

void BtLocalDevice::pairingDone( const QBluetoothAddress &addr,
                                 QBluetoothLocalDevice::Pairing pairing )
{
  lg->info( QString( "BtLocalDevice::pairingDone for %1, return: %2..." )
                .arg( addr.toString() )
                .arg( pairing ) );
  // TODO: signal über Pairing senden
}
