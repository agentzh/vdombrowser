TEMPLATE = app
SOURCES += mainwindow.cpp webpage.cpp main.cpp
HEADERS += mainwindow.h webpage.h
CONFIG -= app_bundle
CONFIG += qt warn_on uitools
DESTDIR = .
#LIBS += -lfcgi

QT+=xml network webkit
QMAKE_RPATHDIR = $$OUTPUT_DIR/lib $$QMAKE_RPATHDIR

CONFIG += $$(QT_WEBKIT)
webkit_trunk {
    WEBKITDIR = $$(WEBKITDIR)
    WEBKITBRANCH = $$(WEBKITBRANCH)
    isEmpty(WEBKITBRANCH) {
        CONFIG(release):WEBKITBUILD = $$WEBKITDIR/WebKitBuild/Release/lib
        CONFIG(debug):WEBKITBUILD = $$WEBKITDIR/WebKitBuild/Debug/lib
    } else {
        CONFIG(release):WEBKITBUILD = $$WEBKITDIR/WebKitBuild/$$WEBKITBRANCH/Release/lib
        CONFIG(debug):WEBKITBUILD = $$WEBKITDIR/WebKitBuild/$$WEBKITBRANCH/Debug/lib
    }
    message(Using WebKit Trunk at $$WEBKITDIR)
    message(Using WebKit Build at $$WEBKITBUILD)
    QT -= webkit
    DEFINES += WEBKIT_TRUNK
    QMAKE_LIBDIR_FLAGS = -L$$WEBKITBUILD
    LIBS = -L$$WEBKITBUILD -lQtWebKit
    INCLUDEPATH = $$WEBKITDIR/WebKit/qt/Api $$INCLUDEPATH
    QMAKE_RPATHDIR = $$WEBKITBUILD $$QMAKE_RPATHDIR
}

