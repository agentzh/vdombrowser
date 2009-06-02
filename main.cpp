#include "version.h"
#include "webpage.h"
#include "mainwindow.h"
#include "urlloader.h"

#include <qwebview.h>
#include <qwebframe.h>
#include <qwebsettings.h>

#include <QtGui>
#include <QDebug>
#include <QPrintPreviewDialog>

#include <QVector>
#include <QTextStream>
#include <QFile>
#include <cstdio>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QString url = QString("%1/%2").arg(QDir::homePath()).arg(QLatin1String("index.html"));

    QWebSettings::setMaximumPagesInCache(20);

    app.setApplicationName(VB_PRODUCT_NAME);
    app.setApplicationVersion(
        QString("%1.%2.%3")
            .arg(VB_MAJORVERSION_NUMBER)
            .arg(VB_MINORVERSION_NUMBER)
            .arg(VB_PATCHLEVEL_NUMBER)
    );
    QCoreApplication::setOrganizationName("Yahoo China EEEE");
    QCoreApplication::setOrganizationDomain("eeeeworks.org");
    QCoreApplication::setApplicationName(VB_PRODUCT_NAME);

    QWebSettings::setObjectCacheCapacities((16*1024*1024) / 8, (16*1024*1024) / 8, 16*1024*1024);

    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);

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

