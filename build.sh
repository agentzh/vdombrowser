#!/bin/sh

export QT_WEBKIT=webkit_trunk
export WEBKITDIR=$HOME/webkit/WebKit
if [ -z "$QTDIR" ]; then
    export QTDIR=/export/vdom/qt
fi
export PATH=$QTDIR/bin:$PATH
    #make distclean
/export/vdom/qt/bin/qmake "CONFIG-=debug" -r
#qmake -r
make -j2

