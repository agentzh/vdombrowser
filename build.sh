#!/bin/sh

#export QT_WEBKIT=webkit_trunk
export WEBKITDIR=
if [ -z "$QTDIR" ]; then
    export QTDIR=/opt/vdom/qt
fi
export PATH=$QTDIR/bin:$PATH
    #make distclean
export QJSON_DIR=/export/vdom/yqjson
$QTDIR/bin/qmake "CONFIG-=debug" "QJSON_DIR=/export/vdom/yqjson" "OUTPUT_DIR=/tmp/a/b" -r
#qmake -r
make -j2

