#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qwebview.h>
#include <qwebframe.h>
#include <qwebsettings.h>
#include <QtGui>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(const QString& url = QString());

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



#endif

