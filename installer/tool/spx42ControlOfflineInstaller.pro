################################################################################
#### Installer erzeugen für das Projekt                                     ####
################################################################################
win32:INSTBINDIR                       = C:/Qt/Tools/QtInstallerFramework/3.0/bin
macx:INSTBINDIR                        = /dev/null

INSTALLERBASE                          = $$INSTBINDIR/installerbase
INSTTOOLBIN                            = $$INSTBINDIR/binarycreator
REPOGEN_BIN                            = $$INSTBINDIR/repogen
CONFIGDIR                              = ../config
PACKAGESDIR                            = ../packages
REPOSITORYS                            = app,qtRuntime
INPUT                                  = $$CONFIGDIR/config.xml
INSTALLER                              = ../spx42ControlOfflineInstaller-0.5.1


TEMPLATE                               = aux
fwInstall.input                        = INPUT
fwInstall.output                       = $$INSTALLER
fwInstall.commands                     = $$INSTTOOLBIN --offline-only -c $$CONFIGDIR/config.xml -i $$REPOSITORYS -p $$PACKAGESDIR ${QMAKE_FILE_OUT}
QMAKE_EXTRA_COMPILERS                 += fwInstall
OTHER_FILES                            = README

