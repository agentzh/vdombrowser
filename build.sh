#!/bin/sh

export QT_WEBKIT=webkit_trunk
export WEBKITDIR=$HOME/webkit/WebKit
#export QTDIR=/opt/qt4.5
export PATH=/opt/qt4.5/bin:$PATH
qmake  "CONFIG-=debug" -r
make clean
make

