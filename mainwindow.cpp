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

