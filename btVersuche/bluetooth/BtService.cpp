#include "BtService.hpp"

BtService::BtService( std::shared_ptr< Logger > logger,
                        const QBluetoothAddress &address,
                        QObject *parent )
    : QObject( parent ), lg( logger )
{
  lg->debug( "BtServices::BtServices..." );
  // Using default Bluetooth adapter
  QBluetoothLocalDevice localDevice;
  QBluetoothAddress adapterAddress = localDevice.address();
  /*
   * In case of multiple Bluetooth adapters it is possible to
   * set which adapter will be used by providing MAC Address.
   * Example code:
   *
   * QBluetoothAddress adapterAddress("XX:XX:XX:XX:XX:XX");
   * discoveryAgent = new QBluetoothServiceDiscoveryAgent(adapterAddress);
   */
  //
  // Service discovery agenten machen
  //
  discoveryAgent = new QBluetoothServiceDiscoveryAgent( adapterAddress );
  discoveryAgent->setRemoteAddress( address );
  //
  // die Signale des Agent mit den Slots verbinden
  //
  connect( discoveryAgent, &QBluetoothServiceDiscoveryAgent::serviceDiscovered,
           this, &BtService::addService );
  connect( discoveryAgent, &QBluetoothServiceDiscoveryAgent::finished, this,
           &BtService::discoverFinished );
  //
  // Agenten starten
  //
  discoveryAgent->start();
}

BtService::~BtService()
{
  delete discoveryAgent;
}

void BtService::addService( const QBluetoothServiceInfo &info )
{
  if ( info.serviceName().isEmpty() )
  {
    lg->warn( "BtServices::addService: service name is empty..." );
    return;
  }
  QString line = info.serviceName();

  if ( !info.serviceDescription().isEmpty() )
    line.append( "\n\t" + info.serviceDescription() );
  if ( !info.serviceProvider().isEmpty() )
    line.append( "\n\t" + info.serviceProvider() );
  lg->info( QString( "BtServices::addService: " ).append( line ) );
}

void BtService::discoverFinished( void )
{
  lg->debug( "BtServices::discoverFinished..." );
}
