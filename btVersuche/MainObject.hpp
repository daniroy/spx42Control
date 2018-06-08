#ifndef MAINOBJECT_HPP
#define MAINOBJECT_HPP

#include <QBluetoothDeviceInfo>
#include <QObject>
#include <QtCore/QCoreApplication>
#include <logging/Logger.hpp>

class MainObject : public QObject
{
  Q_OBJECT
  private:
  std::shared_ptr< Logger > lg;
  bool isRunnning;
  QCoreApplication *ca;

  public:
  explicit MainObject( int argc, char *argv[] );
  ~MainObject();
  int exec( void );

  signals:

  public slots:
  void slotDeviceDiscovered( const QBluetoothDeviceInfo &info );
  void slotDeviceDiscoverFinished( void );
};

#endif  // MAINOBJECT_HPP
