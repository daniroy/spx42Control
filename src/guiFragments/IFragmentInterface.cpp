﻿#include "IFragmentInterface.hpp"

namespace spx
{
  IFragmentInterface::IFragmentInterface( std::shared_ptr< Logger > logger,
                                          std::shared_ptr< SPX42Database > spx42Database,
                                          std::shared_ptr< SPX42Config > spxCfg,
                                          std::shared_ptr< SPX42RemotBtDevice > remSPX42,
                                          AppConfigClass *appCfg )
      : lg( logger ), database( spx42Database ), spxConfig( spxCfg ), remoteSPX42( remSPX42 ), appConfig( appCfg ), ackuVal( 0.0 )
  {
  }
}  // namespace spx
