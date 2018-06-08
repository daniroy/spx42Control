#include <QThread>

#include "MainObject.hpp"
#include "bluetooth/BtLocalDevice.hpp"
#include "bluetooth/BtService.hpp"

MainObject::MainObject( int argc, char *argv[] ) : QObject(), isRunnning( true )
{
  lg = std::shared_ptr< Logger >( new Logger() );
  lg->startLogging( LG_DEBUG, "btversuche.log" );
  lg->debug( "MainObject::MainObject" );
  ca = new QCoreApplication( argc, argv );
}

MainObject::~MainObject()
{
  lg->debug( "MainObject::~MainObject" );
  lg->shutdown();
  delete ca;
}

int MainObject::exec( void )
{
  lg->debug( "MainObject::exec...." );
  //
  // erzeuge lokales device
  //
  BtLocalDevice lbtd( lg, this );
  //
  // verbinde Signale
  //
  connect( &lbtd, &BtLocalDevice::sigFoundDevice, this,
           &MainObject::slotDeviceDiscovered );
  connect( &lbtd, &BtLocalDevice::sigDiscoverFinished, this,
           &MainObject::slotDeviceDiscoverFinished );
  //
  // initiiere einen SCAN
  //
  lbtd.startDeviceScan();
  //
  // übergebe an die Eventschleife und
  // warte auf das seelige Ende
  //
  return ca->exec();
}

void MainObject::slotDeviceDiscovered( const QBluetoothDeviceInfo &info )
{
  lg->debug( QString( "MainObject::slotDeviceDiscovered: device: " )
                 .append( info.address().toString() ) );
}

void MainObject::slotDeviceDiscoverFinished( void )
{
  //
  // leite das ENDE ein
  //
  lg->debug( "MainObject::slotDeviceDiscoverFinished..." );
  ca->quit();
}
