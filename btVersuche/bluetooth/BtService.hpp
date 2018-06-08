#ifndef BTSERVICES_HPP
#define BTSERVICES_HPP

#include <QBluetoothAddress>
#include <QBluetoothLocalDevice>
#include <QBluetoothServiceDiscoveryAgent>
#include <QBluetoothServiceInfo>
#include <QObject>
#include <logging/Logger.hpp>

class BtService : public QObject
{
  Q_OBJECT

  private:
  std::shared_ptr< Logger > lg;

  public:
  explicit BtService( std::shared_ptr< Logger > logger,
                      const QBluetoothAddress &address,
                      QObject *parent = nullptr );
  ~BtService();

  private:
  QBluetoothServiceDiscoveryAgent *discoveryAgent;

  public slots:
  void addService( const QBluetoothServiceInfo &info );
  void discoverFinished( void );
};

#endif  // BTSERVICES_HPP
