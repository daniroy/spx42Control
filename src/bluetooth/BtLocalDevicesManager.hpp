﻿#ifndef BT_LOCAL_DEVICES_MANAGER_HPP
#define BT_LOCAL_DEVICES_MANAGER_HPP

#include <QBluetoothAddress>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QBluetoothLocalDevice>
#include <QObject>
#include <memory>
#include "logging/Logger.hpp"

namespace spx
{
  class BtLocalDevicesManager : public QObject
  {
    Q_OBJECT
    private:
    std::shared_ptr< Logger > lg;
    std::unique_ptr< QBluetoothDeviceDiscoveryAgent > discoveryAgent;
    std::unique_ptr< QBluetoothLocalDevice > localDevice;

    public:
    explicit BtLocalDevicesManager( std::shared_ptr< Logger > logger, QObject *parent = nullptr );
    ~BtLocalDevicesManager();
    void init( void );
    void startDiscoverDevices( void );
    void cancelDiscoverDevices( void );
    const QBluetoothLocalDevice *getLocalDevice( void );
    void setInquiryGeneralUnlimited( bool unlimited );
    void setHostDiscoverable( bool discoverable );
    void setHostPower( bool on );
    void requestPairing( const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing );

    signals:
    void onDiscoveredDeviceSig( const QBluetoothDeviceInfo &info );
    void onDevicePairingDoneSig( const QBluetoothAddress &addr, QBluetoothLocalDevice::Pairing paring );
    void onDiscoverScanFinishedSig( void );
    void onDeviceHostModeStateChangedSig( QBluetoothLocalDevice::HostMode mode );
  };
}
#endif  // BTDEVICES_HPP
