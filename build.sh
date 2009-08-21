#!/bin/sh

#export QT_WEBKIT=webkit_trunk
export WEBKITDIR=
if [ -z "$QTDIR" ]; then
    export QTDIR=/opt/vdom/qt
fi
export PATH=$QTDIR/bin:$PATH
    #make distclean
$QTDIR/bin/qmake "CONFIG-=debug" -r
#qmake -r
make -j2

