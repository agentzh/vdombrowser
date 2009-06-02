#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "lineedit.h"

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

    void changeLocation();

    void loadFinished();

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
    void setupUI();

    void createCentralWidget();
    void createSideBar();
    void createToolBar();
    void createMenus();
    void createProgressBar();
    void createUrlEdit();

    QTextEdit* itemInfoEdit;
    QTextEdit* pageInfoEdit;

    QWebView *view;
    LineEdit *urlEdit;
    QWidget *sidebar;
    QProgressBar *progress;

    QAction *formatMenuAction;

    QStringList urlList;
    QStringListModel urlModel;
};



#endif

