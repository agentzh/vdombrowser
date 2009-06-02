#include "mainwindow.h"
#include "webpage.h"

MainWindow::MainWindow(const QString& url): currentZoom(100) {
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

    QUrl qurl;
    qurl.setEncodedUrl(url.toUtf8(), QUrl::StrictMode);
    if (qurl.isValid()) {
        urlEdit->setText(qurl.toString());
        view->load(qurl);

        // the zoom values are chosen to be like in Mozilla Firefox 3
        zoomLevels << 30 << 50 << 67 << 80 << 90;
        zoomLevels << 100;
        zoomLevels << 110 << 120 << 133 << 150 << 170 << 200 << 240 << 300;
    }
}

void MainWindow::changeLocation() {
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

void MainWindow::loadFinished() {
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

void MainWindow::setupUI() {
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

