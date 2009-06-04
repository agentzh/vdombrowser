#include "mainwindow.h"
#include "webpage.h"

MainWindow::MainWindow(const QString& url): currentZoom(100) {
    QDesktopServices::setUrlHandler(QLatin1String("http"), this, "loadUrl");
    m_hunterConfig = new HunterConfigDialog(this);
    connect(m_hunterConfig, SIGNAL(accepted()),
            this, SLOT(saveHunterConfig()));

    connect(&m_hunter, SIGNAL(readyReadStandardOutput()),
            this, SLOT(emitHunterStdout()));
    connect(&m_hunter, SIGNAL(readyReadStandardError()),
            this, SLOT(emitHunterStderr()));
    connect(&m_hunter, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(hunterFinished(int, QProcess::ExitStatus)));
    connect(&m_hunter, SIGNAL(started()), this, SLOT(hunterStarted()));

    m_settings = new QSettings(
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
        m_urlEdit->setText(qurl.toString());
        m_view->load(qurl);

        // the zoom values are chosen to be like in Mozilla Firefox 3
        zoomLevels << 30 << 50 << 67 << 80 << 90;
        zoomLevels << 100;
        zoomLevels << 110 << 120 << 133 << 150 << 170 << 200 << 240 << 300;
    }
}

void MainWindow::changeLocation() {
    //QUrl url = guessUrlFromString(urlEdit->text());
    //
    QString urlStr = m_urlEdit->text().trimmed();
    QRegExp test(QLatin1String("^[a-zA-Z]+\\:.*"));

    // Check if it looks like a qualified URL. Try parsing it and see.
    bool hasSchema = test.exactMatch(urlStr);
    if (!hasSchema) {
        urlStr = "http://" + urlStr;
    }

    QUrl url;
    url.setEncodedUrl(urlStr.toUtf8(), QUrl::StrictMode);
    m_urlEdit->setText(url.toEncoded());
    m_view->load(url);
    m_view->setFocus(Qt::OtherFocusReason);
}

void MainWindow::loadFinished() {
    m_urlEdit->setText(m_view->url().toEncoded());

    QUrl::FormattingOptions opts;
    opts |= QUrl::RemoveScheme;
    opts |= QUrl::RemoveUserInfo;
    opts |= QUrl::StripTrailingSlash;
    QString s = m_view->url().toString(opts);
    s = s.mid(2);
    if (s.isEmpty())
        return;

    if (!m_urlList.contains(s))
        m_urlList += s;
    m_urlCompleterModel.setStringList(m_urlList);

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
        m_hunterLabel->hide();
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
    //m_hunterConfig->hide();
}

void MainWindow::createCentralWidget() {
    createSideBar();
    createWebView();

    QSplitter* splitter = new QSplitter(this);
    splitter->addWidget(m_view);
    splitter->addWidget(m_sidebar);

    setCentralWidget(splitter);
}

void MainWindow::createWebView() {
    m_view = new QWebView(this);
    QWebPage* page = new QWebPage(m_view);
    page->settings()->setAttribute(QWebSettings::JavascriptEnabled, m_enableJavascript);
    page->settings()->setAttribute(QWebSettings::PluginsEnabled, m_enablePlugins);
    page->settings()->setAttribute(QWebSettings::AutoLoadImages, m_enableImages);
    page->settings()->setAttribute(QWebSettings::JavaEnabled, m_enableJava);

    m_view->setPage(page);
    m_webvdom = new QWebVDom(page->mainFrame());

    connect(m_view, SIGNAL(loadFinished(bool)),
            this, SLOT(loadFinished()));
    connect(m_view, SIGNAL(titleChanged(const QString&)),
            this, SLOT(setWindowTitle(const QString&)));
    connect(m_view->page(), SIGNAL(linkHovered(const QString&, const QString&, const QString &)),
            this, SLOT(showLinkHover(const QString&, const QString&)));
    connect(m_view->page(), SIGNAL(windowCloseRequested()), this, SLOT(deleteLater()));
    connect(m_view, SIGNAL(urlChanged(const QUrl&)), this, SLOT(updateUrl(const QUrl&)));
    connect(m_view, SIGNAL(linkClicked(const QUrl&)), this, SLOT(loadUrl(const QUrl&)));

    m_view->pageAction(QWebPage::Back)->setShortcut(QKeySequence::Back);
    m_view->pageAction(QWebPage::Stop)->setShortcut(Qt::Key_Escape);
    m_view->pageAction(QWebPage::Forward)->setShortcut(QKeySequence::Forward);
    m_view->pageAction(QWebPage::Reload)->setShortcut(Qt::Key_F5);
    m_view->pageAction(QWebPage::Undo)->setShortcut(QKeySequence::Undo);
    m_view->pageAction(QWebPage::Redo)->setShortcut(QKeySequence::Redo);
    m_view->pageAction(QWebPage::Cut)->setShortcut(QKeySequence::Cut);
    m_view->pageAction(QWebPage::Copy)->setShortcut(QKeySequence::Copy);
    m_view->pageAction(QWebPage::Paste)->setShortcut(QKeySequence::Paste);
    m_view->pageAction(QWebPage::ToggleBold)->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));
    m_view->pageAction(QWebPage::ToggleItalic)->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));
    m_view->pageAction(QWebPage::ToggleUnderline)->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_U));
}

void MainWindow::createSideBar() {
    m_sidebar = new QWidget(this);
    QVBoxLayout *sidebarLayout = new QVBoxLayout(m_sidebar);
    m_sidebar->setLayout(sidebarLayout);

    QLabel* label = new QLabel(tr("Active item description"), m_sidebar);
    sidebarLayout->addWidget(label);
    m_itemInfoEdit = new QTextEdit(m_sidebar);
    m_itemInfoEdit->setReadOnly(true);
    sidebarLayout->addWidget(m_itemInfoEdit);

    label = new QLabel(tr("Page item summary"), m_sidebar);
    sidebarLayout->addWidget(label);
    m_pageInfoEdit = new QTextEdit(m_sidebar);
    m_pageInfoEdit->setReadOnly(true);
    sidebarLayout->addWidget(m_pageInfoEdit);
}

void MainWindow::createUrlEdit() {
    m_urlEdit = new LineEdit(this);
    m_urlEdit->setSizePolicy(QSizePolicy::Expanding,
            m_urlEdit->sizePolicy().verticalPolicy());

    connect(m_urlEdit, SIGNAL(returnPressed()),
            SLOT(changeLocation()));

    QCompleter *completer = new QCompleter(this);
    m_urlEdit->setCompleter(completer);
    completer->setModel(&m_urlCompleterModel);
}

void MainWindow::createProgressBar() {
    m_progress = new QProgressBar(this);
    m_progress->setRange(0, 100);
    m_progress->setMinimumSize(100, 20);
    m_progress->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_progress->hide();
    statusBar()->addPermanentWidget(m_progress);

    m_hunterLabel = new QLabel(this);
    m_hunterLabel->hide();
    m_hunterLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    statusBar()->addPermanentWidget(m_hunterLabel);

    connect(m_view, SIGNAL(loadProgress(int)), m_progress, SLOT(show()));
    connect(m_view, SIGNAL(loadProgress(int)), m_progress, SLOT(setValue(int)));
    connect(m_view, SIGNAL(loadFinished(bool)), m_progress, SLOT(hide()));
}

void MainWindow::createToolBar() {
    QToolBar *bar = addToolBar("Navigation");
    bar->addAction(m_view->pageAction(QWebPage::Back));
    bar->addAction(m_view->pageAction(QWebPage::Forward));
    bar->addAction(m_view->pageAction(QWebPage::Reload));
    bar->addAction(m_view->pageAction(QWebPage::Stop));
    bar->addWidget(m_urlEdit);
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
    editMenu->addAction(m_view->pageAction(QWebPage::Undo));
    editMenu->addAction(m_view->pageAction(QWebPage::Redo));
    editMenu->addSeparator();
    editMenu->addAction(m_view->pageAction(QWebPage::Cut));
    editMenu->addAction(m_view->pageAction(QWebPage::Copy));
    editMenu->addAction(m_view->pageAction(QWebPage::Paste));
    //editMenu->addSeparator();
    //QAction *setEditable = editMenu->addAction(tr("Set Editable"), this, SLOT(setEditable(bool)));
    //setEditable->setCheckable(true);
}

void MainWindow::createViewMenu() {
    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(m_view->pageAction(QWebPage::Stop));
    viewMenu->addAction(m_view->pageAction(QWebPage::Reload));
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
    m_settings->beginGroup("MainWindow");

    m_settings->setValue("size", size());
    m_settings->setValue("pos", pos());

    m_settings->setValue("enableJavascript", QVariant(m_enableJavascript));
    m_settings->setValue("enablePlugins", QVariant(m_enablePlugins));
    m_settings->setValue("enableImages", QVariant(m_enableImages));
    m_settings->setValue("enableJava", QVariant(m_enableJava));

    m_settings->setValue("hunterEnabled", QVariant(m_hunterEnabled));
    m_settings->setValue("hunterPath", m_hunterPath);
    m_settings->setValue("vdomPath", m_vdomPath);

    m_settings->endGroup();
}

void MainWindow::readSettings() {
    m_settings->beginGroup("MainWindow");

    resize(m_settings->value("size", QSize(800, 600)).toSize());
    move(m_settings->value("pos", QPoint(200, 200)).toPoint());

    m_enableJavascript = m_settings->value("enableJavascript").toBool();
    m_enablePlugins = m_settings->value("enablePlugins").toBool();
    m_enableImages = m_settings->value("enableImages").toBool();
    m_enableJava = m_settings->value("enableJava").toBool();

    m_hunterEnabled = m_settings->value("hunterEnabled").toBool();
    m_hunterPath = m_settings->value("hunterPath").toString();
    m_vdomPath   = m_settings->value("vdomPath").toString();
    initHunterConfig();

    m_settings->endGroup();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    writeSettings();
    event->accept();
}

void MainWindow::saveHunterConfig() {
    m_hunterEnabled = m_hunterConfig->hunterEnabled();
    //m_hunterPath = m_hunterConfig->progPath();
    m_hunterPath = m_hunterConfig->progPath();
    m_vdomPath   = m_hunterConfig->vdomPath();
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
        return;
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
    //qDebug() << res << endl;
    if (!res.canConvert<QVariantMap>()) {
        QMessageBox::warning(this, tr("Hunter Result File Loader"),
            QString("Result file %1 does not contain a JSON object.")
                .arg(resFile),
            QMessageBox::NoButton);
        return;
    }
    const QVariantMap root = res.toMap();
    annotateWebPage(root);

    QVariant programMeta = root["program"];
    if (!programMeta.isNull() && programMeta.canConvert<QString>()) {
        m_hunterLabel->setText(programMeta.toString());
    } else {
        m_hunterLabel->setText(tr("Unknown hunter"));
    }
    m_hunterLabel->show();

    QVariant jumpTo = root["jump_to"];
    if (!jumpTo.isNull() && jumpTo.canConvert<QVariantMap>()) {
        QVariantMap point = jumpTo.toMap();
        QVariant x = point["x"];
        QVariant y = point["y"];
        if (!x.isNull() && x.canConvert<int>() &&
                !y.isNull() && y.canConvert<int>()) {
            QWebFrame* frame = m_view->page()->mainFrame();
            //qDebug() << "Scroll the page to point (" << x.toInt() <<
                //"," << y.toInt() << ")" << endl;
            frame->scroll(x.toInt(), y.toInt());
        }
    }
}

void MainWindow::annotateWebPage(const QVariant& map) {
    Q_UNUSED(map)
}

