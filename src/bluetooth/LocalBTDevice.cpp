
#include "ABTDevice.hpp"


namespace spx
{
  ABTDevice::ABTDevice( Logger *log, const char *dAddr ) :
    deviceAddr( QByteArray( dAddr )),
    pin("0000"),
    isInitOk( IndicatorStati::OFFLINE ),
    lg( log )
  {
  }

  ABTDevice::ABTDevice( Logger *log, const QByteArray& dAddr) :
    deviceAddr( QByteArray( dAddr )),
    pin("0000"),
    isInitOk( IndicatorStati::OFFLINE ),
    lg( log )
  {
  }


  ABTDevice::ABTDevice(const char *dAddr) :
    deviceAddr( QByteArray( dAddr )),
    pin("0000"),
    isInitOk( IndicatorStati::OFFLINE ),
    lg( new Logger(nullptr) )
  {
  }

  ABTDevice::ABTDevice( const QByteArray& dAddr ) :
    deviceAddr( QByteArray( dAddr )),
    pin("0000"),
    isInitOk( IndicatorStati::OFFLINE ),
    lg( new Logger(nullptr) )
  {
  }

  const QByteArray& ABTDevice::getConnectedDeviceAddr(void)
  {
    return( deviceAddr );
  }

  int ABTDevice::setLogger( Logger * log )
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
