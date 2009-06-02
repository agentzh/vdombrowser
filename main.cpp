#include <qwebpage.h>
#include <qwebview.h>
#include <qwebframe.h>
#include <qwebsettings.h>

#include <QtGui>
#include <QDebug>
#if QT_VERSION >= 0x040400 && !defined(QT_NO_PRINTER)
#include <QPrintPreviewDialog>
#endif

#include <QtUiTools/QUiLoader>

#include <QVector>
#include <QTextStream>
#include <QFile>
#include <cstdio>

class WebPage : public QWebPage
{
public:
    WebPage(QWidget *parent) : QWebPage(parent) {}

    virtual QWebPage *createWindow(QWebPage::WebWindowType);
    virtual QObject* createPlugin(const QString&, const QUrl&, const QStringList&, const QStringList&);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(const QString& url = QString()): currentZoom(100) {
        view = new QWebView(this);
        setCentralWidget(view);

        view->setPage(new WebPage(view));

        connect(view, SIGNAL(loadFinished(bool)),
                this, SLOT(loadFinished()));
        connect(view, SIGNAL(titleChanged(const QString&)),
                this, SLOT(setWindowTitle(const QString&)));
        connect(view->page(), SIGNAL(linkHovered(const QString&, const QString&, const QString &)),
                this, SLOT(showLinkHover(const QString&, const QString&)));
        connect(view->page(), SIGNAL(windowCloseRequested()), this, SLOT(deleteLater()));
        connect(view, SIGNAL(urlChanged(const QUrl&)), this, SLOT(updateUrl(const QUrl&)));
        connect(view, SIGNAL(linkClicked(const QUrl&)), this, SLOT(loadUrl(const QUrl&)));

        setupUI();

        QUrl qurl = guessUrlFromString(url);
        if (qurl.isValid()) {
            urlEdit->setText(qurl.toString());
            view->load(qurl);

            // the zoom values are chosen to be like in Mozilla Firefox 3
            zoomLevels << 30 << 50 << 67 << 80 << 90;
            zoomLevels << 100;
            zoomLevels << 110 << 120 << 133 << 150 << 170 << 200 << 240 << 300;
        }
    }

    QWebPage* webPage() const {
        return view->page();
    }

    QWebView* webView() const {
        return view;
    }

protected slots:

    void loadUrl(const QUrl& url) {
        fprintf(stderr, "Loading new url...");
        view->load(url);
        view->setFocus(Qt::OtherFocusReason);
    }

    void updateUrl(const QUrl& url) {
        urlEdit->setText(url.toEncoded());
    }

    void setWindowTitle(const QString& title) {
        if (title.isNull()) {
            ((QMainWindow*)this)->setWindowTitle("QtLauncher");
        } else {
            ((QMainWindow*)this)->setWindowTitle(QString("QtLauncher - ") + title);
        }
    }

    void selectLineEdit() {
        //fprintf(stderr, "selecting url edit...\n");
        urlEdit->selectAll();
        urlEdit->setFocus();
    }

    void changeLocation() {
        //QUrl url = guessUrlFromString(urlEdit->text());
        //
        QString urlStr = urlEdit->text().trimmed();
        QRegExp test(QLatin1String("^[a-zA-Z]+\\:.*"));

        // Check if it looks like a qualified URL. Try parsing it and see.
        bool hasSchema = test.exactMatch(urlStr);
        if (!hasSchema) {
            urlStr = "http://" + urlStr;
        }

        QUrl url;
        url.setEncodedUrl(urlStr.toUtf8(), QUrl::StrictMode);
        urlEdit->setText(url.toEncoded());
        view->load(url);
        view->setFocus(Qt::OtherFocusReason);
    }

    void loadFinished() {
        urlEdit->setText(view->url().toEncoded());

        QUrl::FormattingOptions opts;
        opts |= QUrl::RemoveScheme;
        opts |= QUrl::RemoveUserInfo;
        opts |= QUrl::StripTrailingSlash;
        QString s = view->url().toString(opts);
        s = s.mid(2);
        if (s.isEmpty())
            return;

        if (!urlList.contains(s))
            urlList += s;
        urlModel.setStringList(urlList);
    }

    void showLinkHover(const QString &link, const QString &toolTip) {
        statusBar()->showMessage(link);
#ifndef QT_NO_TOOLTIP
        if (!toolTip.isEmpty())
            QToolTip::showText(QCursor::pos(), toolTip);
#endif
    }

    void newWindow() {
        MainWindow *mw = new MainWindow;
        mw->show();
    }

    void zoomIn() {
        int i = zoomLevels.indexOf(currentZoom);
        Q_ASSERT(i >= 0);
        if (i < zoomLevels.count() - 1)
            currentZoom = zoomLevels[i + 1];

        view->setZoomFactor(qreal(currentZoom)/100.0);
    }

    void zoomOut() {
        int i = zoomLevels.indexOf(currentZoom);
        Q_ASSERT(i >= 0);
        if (i > 0)
            currentZoom = zoomLevels[i - 1];

        view->setZoomFactor(qreal(currentZoom)/100.0);
    }

    void resetZoom()
    {
       currentZoom = 100;
       view->setZoomFactor(1.0);
    }

    void toggleZoomTextOnly(bool b)
    {
        view->page()->settings()->setAttribute(QWebSettings::ZoomTextOnly, b);
    }

    void print() {
#if QT_VERSION >= 0x040400 && !defined(QT_NO_PRINTER)
        QPrintPreviewDialog dlg(this);
        connect(&dlg, SIGNAL(paintRequested(QPrinter *)),
                view, SLOT(print(QPrinter *)));
        dlg.exec();
#endif
    }

    void setEditable(bool on) {
        view->page()->setContentEditable(on);
        formatMenuAction->setVisible(on);
    }

    void dumpHtml() {
        qDebug() << "HTML: " << view->page()->mainFrame()->toHtml();
    }
private:

    QVector<int> zoomLevels;
    int currentZoom;

    // create the status bar, tool bar & menu
    void setupUI() {
        progress = new QProgressBar(this);
        progress->setRange(0, 100);
        progress->setMinimumSize(100, 20);
        progress->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        progress->hide();
        statusBar()->addPermanentWidget(progress);

        connect(view, SIGNAL(loadProgress(int)), progress, SLOT(show()));
        connect(view, SIGNAL(loadProgress(int)), progress, SLOT(setValue(int)));
        connect(view, SIGNAL(loadFinished(bool)), progress, SLOT(hide()));

        urlEdit = new QLineEdit(this);
        urlEdit->setSizePolicy(QSizePolicy::Expanding, urlEdit->sizePolicy().verticalPolicy());
        connect(urlEdit, SIGNAL(returnPressed()),
                SLOT(changeLocation()));
        QCompleter *completer = new QCompleter(this);
        urlEdit->setCompleter(completer);
        completer->setModel(&urlModel);

        QToolBar *bar = addToolBar("Navigation");
        bar->addAction(view->pageAction(QWebPage::Back));
        bar->addAction(view->pageAction(QWebPage::Forward));
        bar->addAction(view->pageAction(QWebPage::Reload));
        bar->addAction(view->pageAction(QWebPage::Stop));
        bar->addWidget(urlEdit);

        QMenu *fileMenu = menuBar()->addMenu("&File");
        QAction *newWindow = fileMenu->addAction("New Window", this, SLOT(newWindow()));
#if QT_VERSION >= 0x040400
        fileMenu->addAction(tr("Print"), this, SLOT(print()));
#endif
        fileMenu->addAction("Close", this, SLOT(close()));

        QMenu *editMenu = menuBar()->addMenu("&Edit");
        editMenu->addAction(view->pageAction(QWebPage::Undo));
        editMenu->addAction(view->pageAction(QWebPage::Redo));
        editMenu->addSeparator();
        editMenu->addAction(view->pageAction(QWebPage::Cut));
        editMenu->addAction(view->pageAction(QWebPage::Copy));
        editMenu->addAction(view->pageAction(QWebPage::Paste));
        editMenu->addSeparator();
        QAction *setEditable = editMenu->addAction("Set Editable", this, SLOT(setEditable(bool)));
        setEditable->setCheckable(true);

        QMenu *viewMenu = menuBar()->addMenu("&View");
        viewMenu->addAction(view->pageAction(QWebPage::Stop));
        viewMenu->addAction(view->pageAction(QWebPage::Reload));
        viewMenu->addSeparator();
        QAction *zoomIn = viewMenu->addAction("Zoom &In", this, SLOT(zoomIn()));
        QAction *zoomOut = viewMenu->addAction("Zoom &Out", this, SLOT(zoomOut()));
        QAction *resetZoom = viewMenu->addAction("Reset Zoom", this, SLOT(resetZoom()));
        QAction *zoomTextOnly = viewMenu->addAction("Zoom Text Only", this, SLOT(toggleZoomTextOnly(bool)));
        zoomTextOnly->setCheckable(true);
        zoomTextOnly->setChecked(false);
        viewMenu->addSeparator();
        viewMenu->addAction("Dump HTML", this, SLOT(dumpHtml()));

        QMenu *formatMenu = new QMenu("F&ormat");
        formatMenuAction = menuBar()->addMenu(formatMenu);
        formatMenuAction->setVisible(false);
        formatMenu->addAction(view->pageAction(QWebPage::ToggleBold));
        formatMenu->addAction(view->pageAction(QWebPage::ToggleItalic));
        formatMenu->addAction(view->pageAction(QWebPage::ToggleUnderline));
        QMenu *writingMenu = formatMenu->addMenu(tr("Writing Direction"));
        writingMenu->addAction(view->pageAction(QWebPage::SetTextDirectionDefault));
        writingMenu->addAction(view->pageAction(QWebPage::SetTextDirectionLeftToRight));
        writingMenu->addAction(view->pageAction(QWebPage::SetTextDirectionRightToLeft));

        newWindow->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
        view->pageAction(QWebPage::Back)->setShortcut(QKeySequence::Back);
        view->pageAction(QWebPage::Stop)->setShortcut(Qt::Key_Escape);
        view->pageAction(QWebPage::Forward)->setShortcut(QKeySequence::Forward);
        view->pageAction(QWebPage::Reload)->setShortcut(QKeySequence::Refresh);
        view->pageAction(QWebPage::Undo)->setShortcut(QKeySequence::Undo);
        view->pageAction(QWebPage::Redo)->setShortcut(QKeySequence::Redo);
        view->pageAction(QWebPage::Cut)->setShortcut(QKeySequence::Cut);
        view->pageAction(QWebPage::Copy)->setShortcut(QKeySequence::Copy);
        view->pageAction(QWebPage::Paste)->setShortcut(QKeySequence::Paste);
        zoomIn->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus));
        zoomOut->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));
        resetZoom->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
        view->pageAction(QWebPage::ToggleBold)->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));
        view->pageAction(QWebPage::ToggleItalic)->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));
        view->pageAction(QWebPage::ToggleUnderline)->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_U));

        QAction* focusAddressBar = new QAction(this);
        // Add the location bar shortcuts familiar to users from other browsers
        QList<QKeySequence> editUrlShortcuts;
        editUrlShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::Key_L));
        editUrlShortcuts.append(QKeySequence(Qt::AltModifier + Qt::Key_O));
        editUrlShortcuts.append(QKeySequence(Qt::AltModifier + Qt::Key_D));
        focusAddressBar->setShortcuts(editUrlShortcuts);
        connect(focusAddressBar, SIGNAL(triggered()),
                this, SLOT(selectLineEdit()));
        fileMenu->addAction(focusAddressBar);
    }

    QUrl guessUrlFromString(const QString &string) {
        QString urlStr = string.trimmed();
        QRegExp test(QLatin1String("^[a-zA-Z]+\\:.*"));

        // Check if it looks like a qualified URL. Try parsing it and see.
        bool hasSchema = test.exactMatch(urlStr);
        if (hasSchema) {
            QUrl url(urlStr, QUrl::TolerantMode);
            if (url.isValid())
                return url;
        }

        // Might be a file.
        if (QFile::exists(urlStr))
            return QUrl::fromLocalFile(urlStr);

        // Might be a shorturl - try to detect the schema.
        if (!hasSchema) {
            int dotIndex = urlStr.indexOf(QLatin1Char('.'));
            if (dotIndex != -1) {
                QString prefix = urlStr.left(dotIndex).toLower();
                QString schema = (prefix == QLatin1String("ftp")) ? prefix : QLatin1String("http");
                QUrl url(schema + QLatin1String("://") + urlStr, QUrl::TolerantMode);
                if (url.isValid())
                    return url;
            }
        }

        // Fall back to QUrl's own tolerant parser.
        return QUrl(string, QUrl::TolerantMode);
    }

    QWebView *view;
    QLineEdit *urlEdit;
    QProgressBar *progress;

    QAction *formatMenuAction;

    QStringList urlList;
    QStringListModel urlModel;
};

QWebPage *WebPage::createWindow(QWebPage::WebWindowType)
{
    MainWindow *mw = new MainWindow;
    mw->show();
    return mw->webPage();
}

QObject *WebPage::createPlugin(const QString &classId, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues)
{
    Q_UNUSED(url);
    Q_UNUSED(paramNames);
    Q_UNUSED(paramValues);
    QUiLoader loader;
    return loader.createWidget(classId, view());
}

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
