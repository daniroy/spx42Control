
#include "LocalBTDevice.hpp"


namespace spx
{
  /**
   * @brief Konstruktor BTDevice (explicit!)
   * @param loggerobjet obligatorisch
   */
  LocalBTDevice::LocalBTDevice( Logger *log ) :
    pin("0000"),
    isInitOk( IndicatorStati::OFFLINE ),
    lg( log ),
    localAdapters(QBluetoothLocalDevice::allDevices()),
    discoveryAgent(nullptr)
  {
    //
    // reagiere auf Anzahl der gefunden Adapter
    //
    if( localAdapters.count() > 0 )
    {
      lg->debug( QString("BTDevice::BTDevice: first BT adapter <").append(localAdapters.at(0).address().toString()).append(">");
      // falls ich gefunden werden will...
      // QBluetoothLocalDevice adapter(localAdapters.at(0).address());
      // adapter.setHostMode(QBluetoothLocalDevice::HostDiscoverable);
    }
    else
    {
      lg->crit(QString("BTDevice::BTDevice: there is no BT Adapter in the system!"));
    }
  }

  /**
   * @brief BTDevice::~BTDevice Der DESTRUKTOR
   */
  LocalBTDevice::~LocalBTDevice()
  {
    if( discoveryAgent != nullptr )
    {
      delete discoveryAgent;
    }
  }

  /**
   * @brief BTDevice::getBtHostInfo gib Hostinfo des indizierten BT Adapters zurück
   * @param adapterIndex Adapter index oder 0
   * @return Hostinfo oder NULL
   */
  QBluetoothHostInfo LocalBTDevice::getBtHostInfo( int adapterIndex )
  {
    //
    // sicherstellen dass der im erlaubten Bereich liegt und vorhanden ist
    //
    if( localAdapters.count() > 0 && adapterIndex >= 0 && adapterIndex < localAdapters.count())
    {
      lg->debug(QString("BTDevice::getBtHostInfo: get hostinfo for internal adapter ").append(adapterIndex));
      return( localAdapters.at(adapterIndex) );
    }
    lg->warn("BTDevice::getBtHostInfo: hostinfo index out of index or no bt adapter in system.");
    return( nullptr );
  }

  /**
   * @brief BTDevice::discoverDevices finde BT Geräte
   * @param uuid die UUID des gesuchten Services
   */
  void LocalBTDevice::discoverDevices(const QBluetoothUuid &uuid)
  {
    lg->debug( QString("discovering bt devices..."));
    //
    // stelle eine lokale BT Adresse zur Verfügung
    //
    const QBluetoothAddress adapter = localAdapters.isEmpty() ? QBluetoothAddress() : localAdapters.at(currentAdapterIndex).address();
    //
    // discoveryagenten erzeugen
    //
    discoveryAgent = new QBluetoothServiceDiscoveryAgent(adapter);
    try
    {
      //connect(discoveryAgent, SIGNAL(serviceDiscovered(QBluetoothServiceInfo)), this, SLOT(serviceDiscovered(QBluetoothServiceInfo)));
      connect(discoveryAgent, &LocalBTDevice::serviceDiscovered, this, &LocalBTDevice::serviceDiscovered);
      connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::finished, this, &LocalBTDevice::discoveryFinished);
    }
    catch(std::exception ex )
    {
      lg->crit( QString("BTDevice::discoverDevices: exception while connecting signals to slots (").append(ex.what()).append( ")"));
      disconnect(discoveryAgent, &LocalBTDevice::serviceDiscovered, this, &LocalBTDevice::serviceDiscovered);
      disconnect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::finished, this, &LocalBTDevice::discoveryFinished);
      discoveryFinished();
      return;
    }
    //
    // signalisiere den Beginn des Discovering
    //
    emit btDiscoveringSig();
    if( discoveryAgent->isActive() )
    {
      // für den Fall, dass da schon was aktiv ist...
      discoveryAgent->stop();
    }
    discoveryAgent->setUuidFilter(uuid);
    discoveryAgent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);
  }

  /**
   * @brief LocalBTDevice::stopDiscoverDevices
   */
  void LocalBTDevice::stopDiscoverDevices(void)
  {
    if( discoveryAgent )
    {
      discoveryAgent->stop();
    }
  }

  /**
   * @brief LocalBTDevice::serviceDiscovered Beim suchen einen Service gefunden....
   * @param serviceInfo
   */
  void LocalBTDevice::serviceDiscovered(const QBluetoothServiceInfo &serviceInfo)
  {
    lg->debug(QString("BTDevice::serviceDiscovered: service addr: ").append(serviceInfo.device().address().toString()));
    lg->debug(QString("BTDevice::serviceDiscovered: service name: ").append(serviceInfo.serviceName()));
    lg->debug(QString("BTDevice::serviceDiscovered: RFCOMM Channel: ").append(serviceInfo.serverChannel()));
    //
    // signal auslösen
    //
    emit btDeviceDiscoveredSig(serviceInfo);
  }

  /**
   * @brief LocalBTDevice::discoveryFinished Suche ist beendet!
   */
  void LocalBTDevice::discoveryFinished()
  {
    disconnect(discoveryAgent, &LocalBTDevice::serviceDiscovered, this, &LocalBTDevice::serviceDiscovered);
    disconnect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::finished, this, &LocalBTDevice::discoveryFinished);
    delete discoveryAgent;
    discoveryAgent = nullptr;
    emit btDiscoverEndSig();)
  }


  const QByteArray& LocalBTDevice::getConnectedDeviceAddr(void)
  {
    return( deviceAddr );
  }

  int LocalBTDevice::setLogger( Logger * log )
  {
    if( lg != nullptr )
    {
      lg->shutdown();
      delete( lg );
      lg = nullptr;
    }
    lg = log;
    return( 0 );
  }
}
