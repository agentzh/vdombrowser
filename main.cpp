#include <qwebview.h>
#include <qwebframe.h>
#include <qwebsettings.h>

#include <QtGui>
#include <QDebug>
#if QT_VERSION >= 0x040400 && !defined(QT_NO_PRINTER)
#include <QPrintPreviewDialog>
#endif

#include <QVector>
#include <QTextStream>
#include <QFile>
#include <cstdio>

#include "webpage.h"
#include "mainwindow.h"
#include "urlloader.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QString url = QString("%1/%2").arg(QDir::homePath()).arg(QLatin1String("index.html"));

    QWebSettings::setMaximumPagesInCache(4);

    app.setApplicationName("QtLauncher");
#if QT_VERSION >= 0x040400
    app.setApplicationVersion("0.1");
#endif

    QWebSettings::setObjectCacheCapacities((16*1024*1024) / 8, (16*1024*1024) / 8, 16*1024*1024);

    QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, false);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::AutoLoadImages, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::JavaEnabled, false);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::JavascriptEnabled, true);

    const QStringList args = app.arguments();

    // robotized
    if (args.contains(QLatin1String("-r"))) {
        QString listFile = args.at(2);
        if (!(args.count() == 3) && QFile::exists(listFile)) {
            qDebug() << "Usage: QtLauncher -r listfile";
            exit(0);
        }
        MainWindow window(url);
        QWebView *view = window.webView();
        URLLoader loader(view, listFile);
        QObject::connect(view, SIGNAL(loadFinished(bool)), &loader, SLOT(loadNext()));
        window.show();
        return app.exec();
    } else {
        if (args.count() > 1)
            url = args.at(1);

        MainWindow window(url);
        window.show();
        return app.exec();
    }
}

