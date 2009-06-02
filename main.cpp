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

class URLLoader : public QObject
{
    Q_OBJECT
public:
    URLLoader(QWebView* view, const QString& inputFileName)
        : m_view(view)
        , m_stdOut(stdout)
    {
        init(inputFileName);
    }

public slots:
    void loadNext()
    {
        QString qstr;
        if (getUrl(qstr)) {
            QUrl url;
            url.setEncodedUrl(qstr.toUtf8(), QUrl::StrictMode);
            if (url.isValid()) {
                m_stdOut << "Loading " << qstr << " ......" << endl;
                m_view->load(url);
            } else
                loadNext();
        } else
            disconnect(m_view, 0, this, 0);
    }

private:
    void init(const QString& inputFileName)
    {
        QFile inputFile(inputFileName);
        if (inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&inputFile);
            QString line;
            while (true) {
                line = stream.readLine();
                if (line.isNull())
                    break;
                m_urls.append(line);
            }
        } else {
            qDebug() << "Cant't open list file";
            exit(0);
        }
        m_index = 0;
        inputFile.close();
    }

    bool getUrl(QString& qstr)
    {
        if (m_index == m_urls.size())
            return false;

        qstr = m_urls[m_index++];
        return true;
    }

private:
    QVector<QString> m_urls;
    int m_index;
    QWebView* m_view;
    QTextStream m_stdOut;
};

#include "main.moc"

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
