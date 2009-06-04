#include "mainwindow.h"
#include "webpage.h"

MainWindow::MainWindow(const QString& url): currentZoom(100) {
    QDesktopServices::setUrlHandler(QLatin1String("http"), this, "loadUrl");
    hunterConfig = new HunterConfigDialog(this);
    connect(hunterConfig, SIGNAL(accepted()),
            this, SLOT(saveHunterConfig()));

    connect(&m_hunter, SIGNAL(readyReadStandardOutput()),
            this, SLOT(emitHunterStdout()));
    connect(&m_hunter, SIGNAL(readyReadStandardError()),
            this, SLOT(emitHunterStderr()));
    connect(&m_hunter, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(hunterFinished(int, QProcess::ExitStatus)));
    connect(&m_hunter, SIGNAL(started()), this, SLOT(hunterStarted()));

    settings = new QSettings(
        QSettings::UserScope,
        qApp->organizationDomain(),
        qApp->applicationName(),
        this
    );
    readSettings();

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

    if (m_hunterEnabled) {
        /* dump VDOM to the external file */
        const QByteArray& vdom = m_webvdom->dump();
        //qDebug() << QString::fromUtf8(vdom);
        QFile file(m_vdomPath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, tr("VDOM Dumper"),
                QString("Failed to open file ") +
                m_vdomPath + " for writing: " +
                file.errorString(), QMessageBox::NoButton);
            return;
        }
        if (file.write(vdom) == -1) {
            QMessageBox::warning(this, tr("VDOM Dumper"),
                QString("Failed to write VDOM dump to file ") +
                m_vdomPath + ": " +
                file.errorString(), QMessageBox::NoButton);
            file.close();
            return;
        }
        file.close();

        /* execute the external hunter program */
        m_hunter.close();
        QStringList args;
        args << m_vdomPath;
        m_itemInfoEdit->clear();
        m_pageInfoEdit->clear();
        statusBar()->showMessage("Starting " + m_hunterPath + "...");
        m_hunter.start(m_hunterPath, args);
    }
}

void MainWindow::setupUI() {
    createCentralWidget();
    createProgressBar();
    createUrlEdit();
    createToolBar();
    createMenus();
    //hunterConfig->hide();
}

void MainWindow::createCentralWidget() {
    createSideBar();
    createWebView();

    QSplitter* splitter = new QSplitter(this);
    splitter->addWidget(view);
    splitter->addWidget(sidebar);

    setCentralWidget(splitter);
}

void MainWindow::createWebView() {
    view = new QWebView(this);
    QWebPage* page = new QWebPage(view);
    page->settings()->setAttribute(QWebSettings::JavascriptEnabled, m_enableJavascript);
    page->settings()->setAttribute(QWebSettings::PluginsEnabled, m_enablePlugins);
    page->settings()->setAttribute(QWebSettings::AutoLoadImages, m_enableImages);
    page->settings()->setAttribute(QWebSettings::JavaEnabled, m_enableJava);

    view->setPage(page);
    m_webvdom = new QWebVDom(page->mainFrame());

    connect(view, SIGNAL(loadFinished(bool)),
            this, SLOT(loadFinished()));
    connect(view, SIGNAL(titleChanged(const QString&)),
            this, SLOT(setWindowTitle(const QString&)));
    connect(view->page(), SIGNAL(linkHovered(const QString&, const QString&, const QString &)),
            this, SLOT(showLinkHover(const QString&, const QString&)));
    connect(view->page(), SIGNAL(windowCloseRequested()), this, SLOT(deleteLater()));
    connect(view, SIGNAL(urlChanged(const QUrl&)), this, SLOT(updateUrl(const QUrl&)));
    connect(view, SIGNAL(linkClicked(const QUrl&)), this, SLOT(loadUrl(const QUrl&)));

    view->pageAction(QWebPage::Back)->setShortcut(QKeySequence::Back);
    view->pageAction(QWebPage::Stop)->setShortcut(Qt::Key_Escape);
    view->pageAction(QWebPage::Forward)->setShortcut(QKeySequence::Forward);
    view->pageAction(QWebPage::Reload)->setShortcut(Qt::Key_F5);
    view->pageAction(QWebPage::Undo)->setShortcut(QKeySequence::Undo);
    view->pageAction(QWebPage::Redo)->setShortcut(QKeySequence::Redo);
    view->pageAction(QWebPage::Cut)->setShortcut(QKeySequence::Cut);
    view->pageAction(QWebPage::Copy)->setShortcut(QKeySequence::Copy);
    view->pageAction(QWebPage::Paste)->setShortcut(QKeySequence::Paste);
    view->pageAction(QWebPage::ToggleBold)->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));
    view->pageAction(QWebPage::ToggleItalic)->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));
    view->pageAction(QWebPage::ToggleUnderline)->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_U));
}

void MainWindow::createSideBar() {
    sidebar = new QWidget(this);
    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);
    sidebar->setLayout(sidebarLayout);

    QLabel* label = new QLabel(tr("Active item description"), sidebar);
    sidebarLayout->addWidget(label);
    m_itemInfoEdit = new QTextEdit(sidebar);
    m_itemInfoEdit->setReadOnly(true);
    sidebarLayout->addWidget(m_itemInfoEdit);

    label = new QLabel(tr("Page item summary"), sidebar);
    sidebarLayout->addWidget(label);
    m_pageInfoEdit = new QTextEdit(sidebar);
    m_pageInfoEdit->setReadOnly(true);
    sidebarLayout->addWidget(m_pageInfoEdit);
}

void MainWindow::createUrlEdit() {
    urlEdit = new LineEdit(this);
    urlEdit->setSizePolicy(QSizePolicy::Expanding, urlEdit->sizePolicy().verticalPolicy());

    connect(urlEdit, SIGNAL(returnPressed()),
            SLOT(changeLocation()));

    QCompleter *completer = new QCompleter(this);
    urlEdit->setCompleter(completer);
    completer->setModel(&urlModel);
}

void MainWindow::createProgressBar() {
    progress = new QProgressBar(this);
    progress->setRange(0, 100);
    progress->setMinimumSize(100, 20);
    progress->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    progress->hide();
    statusBar()->addPermanentWidget(progress);

    connect(view, SIGNAL(loadProgress(int)), progress, SLOT(show()));
    connect(view, SIGNAL(loadProgress(int)), progress, SLOT(setValue(int)));
    connect(view, SIGNAL(loadFinished(bool)), progress, SLOT(hide()));
}

void MainWindow::createToolBar() {
    QToolBar *bar = addToolBar("Navigation");
    bar->addAction(view->pageAction(QWebPage::Back));
    bar->addAction(view->pageAction(QWebPage::Forward));
    bar->addAction(view->pageAction(QWebPage::Reload));
    bar->addAction(view->pageAction(QWebPage::Stop));
    bar->addWidget(urlEdit);
}

void MainWindow::createMenus() {
    createFileMenu();
    createEditMenu();
    createViewMenu();
    createSettingsMenu();
    createHelpMenu();
}

void MainWindow::createFileMenu() {
    QMenu *fileMenu = menuBar()->addMenu("&File");
    QAction *newWindow = fileMenu->addAction(tr("New Window"), this, SLOT(newWindow()));
    newWindow->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));

    QAction* focusAddressBar = new QAction(tr("Open Location..."), this);
    // Add the location bar shortcuts familiar to users from other browsers
    QList<QKeySequence> editUrlShortcuts;
    editUrlShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::Key_L));
    editUrlShortcuts.append(QKeySequence(Qt::AltModifier + Qt::Key_O));
    editUrlShortcuts.append(QKeySequence(Qt::AltModifier + Qt::Key_D));
    focusAddressBar->setShortcuts(editUrlShortcuts);
    connect(focusAddressBar, SIGNAL(triggered()),
            this, SLOT(selectLineEdit()));
    fileMenu->addAction(focusAddressBar);

    fileMenu->addAction(tr("Print"), this, SLOT(print()));
    fileMenu->addAction(tr("Close"), this, SLOT(close()));
}

void MainWindow::createEditMenu() {
    QMenu *editMenu = menuBar()->addMenu("&Edit");
    editMenu->addAction(view->pageAction(QWebPage::Undo));
    editMenu->addAction(view->pageAction(QWebPage::Redo));
    editMenu->addSeparator();
    editMenu->addAction(view->pageAction(QWebPage::Cut));
    editMenu->addAction(view->pageAction(QWebPage::Copy));
    editMenu->addAction(view->pageAction(QWebPage::Paste));
    //editMenu->addSeparator();
    //QAction *setEditable = editMenu->addAction(tr("Set Editable"), this, SLOT(setEditable(bool)));
    //setEditable->setCheckable(true);
}

void MainWindow::createViewMenu() {
    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(view->pageAction(QWebPage::Stop));
    viewMenu->addAction(view->pageAction(QWebPage::Reload));
    viewMenu->addSeparator();

    QAction *zoomIn = viewMenu->addAction(tr("Zoom &In"), this, SLOT(zoomIn()));
    zoomIn->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus));

    QAction *zoomOut = viewMenu->addAction(tr("Zoom &Out"), this, SLOT(zoomOut()));
    zoomOut->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));

    QAction *resetZoom = viewMenu->addAction(tr("Reset Zoom"), this, SLOT(resetZoom()));
    resetZoom->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));

    QAction *zoomTextOnly = viewMenu->addAction(tr("Zoom Text Only"), this, SLOT(toggleZoomTextOnly(bool)));
    zoomTextOnly->setCheckable(true);
    zoomTextOnly->setChecked(false);

    //viewMenu->addSeparator();
    //viewMenu->addAction("Dump HTML", this, SLOT(dumpHtml()));
}

void MainWindow::createSettingsMenu() {
    QMenu *prefMenu = menuBar()->addMenu(tr("&Preferences"));

    {
        QAction *enableJS = prefMenu->addAction(tr("Enable &Javascript"), this, SLOT(toggleEnableJavascript(bool)));
        enableJS->setCheckable(true);
        enableJS->setChecked(m_enableJavascript);
    }
    {
        QAction *enableImages = prefMenu->addAction(tr("Enable &Images"), this, SLOT(toggleEnableImages(bool)));
        enableImages->setCheckable(true);
        enableImages->setChecked(m_enableImages);
    }
    {
        QAction *enablePlugins = prefMenu->addAction(tr("Enable &Plugins"), this, SLOT(toggleEnablePlugins(bool)));
        enablePlugins->setCheckable(true);
        enablePlugins->setChecked(m_enablePlugins);
    }
    {
        QAction *enableJava = prefMenu->addAction(tr("Enable J&ava"), this, SLOT(toggleEnableJava(bool)));
        enableJava->setCheckable(true);
        enableJava->setChecked(m_enableJava);
    }
    prefMenu->addSeparator();
    prefMenu->addAction(tr("X &Hunter"), this, SLOT(execHunterConfig()));
}

void MainWindow::createHelpMenu() {
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(tr("About &Qt"), qApp, SLOT(aboutQt()));
    helpMenu->addAction(tr("&About ") + qApp->applicationName(),
            this, SLOT(aboutMe()));
}

void MainWindow::writeSettings() {
    settings->beginGroup("MainWindow");

    settings->setValue("size", size());
    settings->setValue("pos", pos());

    settings->setValue("enableJavascript", QVariant(m_enableJavascript));
    settings->setValue("enablePlugins", QVariant(m_enablePlugins));
    settings->setValue("enableImages", QVariant(m_enableImages));
    settings->setValue("enableJava", QVariant(m_enableJava));

    settings->setValue("hunterEnabled", QVariant(m_hunterEnabled));
    settings->setValue("hunterPath", m_hunterPath);
    settings->setValue("vdomPath", m_vdomPath);

    settings->endGroup();
}

void MainWindow::readSettings() {
    settings->beginGroup("MainWindow");

    resize(settings->value("size", QSize(800, 600)).toSize());
    move(settings->value("pos", QPoint(200, 200)).toPoint());

    m_enableJavascript = settings->value("enableJavascript").toBool();
    m_enablePlugins = settings->value("enablePlugins").toBool();
    m_enableImages = settings->value("enableImages").toBool();
    m_enableJava = settings->value("enableJava").toBool();

    m_hunterEnabled = settings->value("hunterEnabled").toBool();
    m_hunterPath = settings->value("hunterPath").toString();
    m_vdomPath   = settings->value("vdomPath").toString();
    initHunterConfig();

    settings->endGroup();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    writeSettings();
    event->accept();
}

void MainWindow::saveHunterConfig() {
    m_hunterEnabled = hunterConfig->hunterEnabled();
    //m_hunterPath = hunterConfig->progPath();
    m_hunterPath = hunterConfig->progPath();
    m_vdomPath   = hunterConfig->vdomPath();
    //qDebug() << "Saving hunter config... (hunter: " << m_hunterPath << ")";
}

void MainWindow::hunterFinished(int exitCode, QProcess::ExitStatus) {
    if (exitCode != 0) {
        QString msg = QString("Failed to spawn X Hunter %1: %2: "
                "Process returns exit code %3.")
                .arg(m_hunterPath)
                .arg(m_hunter.errorString())
                .arg(m_hunter.exitCode());
        m_itemInfoEdit->append(msg);
        QMessageBox::warning(this, tr("Hunter runner"),
                msg, QMessageBox::NoButton);
        return;
    }
    statusBar()->showMessage(
        QString("Finished running X Hunter %1. (exit code: %2)")
                .arg(m_hunterPath).arg(exitCode));

    /* Process the .res output file by hunter programs */
    QString resFile = m_vdomPath + ".res";
    if (!QFile::exists(resFile)) {
        QMessageBox::warning(this, tr("Hunter Result File Checker"),
            QString("Hunter result data file \"%1\"not found."),
            QMessageBox::NoButton);
        return;
    }

    QFile file(resFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Hunter Result File Loader"),
            QString("Failed to load hunter result file %1: %2")
                .arg(resFile).arg(file.errorString()),
                QMessageBox::NoButton);
        file.close();
        return;
    }
    QString json = QString::fromUtf8(file.readAll());
    file.close();
    if (json.isEmpty()) {
        QMessageBox::warning(this, tr("Hunter Result File Loader"),
            QString("Result file %1: %2")
                .arg(m_vdomPath).arg(file.errorString()),
                QMessageBox::NoButton);

    }

    bool status = true;
    QVariant res = m_jsonDriver.parse(json, &status);
    if (status) {
        QMessageBox::warning(this, tr("Hunter Result File Loader"),
            QString("Failed to parse JSON in file %1: line %2: %3")
                .arg(resFile)
                .arg(m_jsonDriver.errorLine())
                .arg(m_jsonDriver.error()),
            QMessageBox::NoButton);
        return;
    }
    qDebug() << res << endl;
}

