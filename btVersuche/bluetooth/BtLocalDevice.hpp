#ifndef BTLOCALDEVICE_HPP
#define BTLOCALDEVICE_HPP

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QBluetoothLocalDevice>
#include <QObject>
#include <logging/Logger.hpp>

// QT_FORWARD_DECLARE_CLASS( QBluetoothDeviceDiscoveryAgent )
// QT_FORWARD_DECLARE_CLASS( QBluetoothDeviceInfo )

QT_USE_NAMESPACE

class BtLocalDevice : public QObject
{
  Q_OBJECT

  private:
  std::shared_ptr< Logger > lg;

  public:
  explicit BtLocalDevice( std::shared_ptr< Logger > logger,
                          QObject *parent = nullptr );
  ~BtLocalDevice();
  void startDeviceScan( void );

  signals:
  void sigDiscoverStarted( void );
  void sigFoundDevice( const QBluetoothDeviceInfo &info );
  void sigDiscoverFinished( void );

  public slots:
  void addDevice( const QBluetoothDeviceInfo &info );
  void pairingDone( const QBluetoothAddress &addr,
                    QBluetoothLocalDevice::Pairing pairing );

  private slots:
  void scanFinished( void );
  void hostModeStateChanged( QBluetoothLocalDevice::HostMode );

  private:
  QBluetoothDeviceDiscoveryAgent *discoveryAgent;
  QBluetoothLocalDevice *localDevice;
};

#endif  // BTLOCALDEVICE_HPP
