#!/bin/sh

export QT_WEBKIT=webkit_trunk
export WEBKITDIR=$HOME/webkit/WebKit
if [ -z "$QTDIR" ]; then
    export QTDIR=/opt/qt4.5
fi
export PATH=$QTDIR/bin:$PATH
    #make distclean
/opt/qt4.5/bin/qmake "CONFIG-=debug" -r
#qmake -r
make -j2

