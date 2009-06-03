#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "lineedit.h"
#include "aboutdialog.h"
#include "hunterconfigdialog.h"

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

protected:

    virtual void closeEvent(QCloseEvent * event);

protected slots:

    void loadUrl(const QUrl& url) {
        //fprintf(stderr, "Loading new url...");
        view->load(url);
        view->setFocus(Qt::OtherFocusReason);
    }

    void updateUrl(const QUrl& url) {
        urlEdit->setText(url.toEncoded());
    }

    void setWindowTitle(const QString& title) {
        if (title.isNull()) {
            ((QMainWindow*)this)->setWindowTitle(qApp->applicationName());
        } else {
            ((QMainWindow*)this)->setWindowTitle(
                qApp->applicationName() + " - " + title);
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

    void aboutMe() {
        AboutDialog* about = new AboutDialog(this);
        about->show();
    }

    void toggleEnableJavascript(bool enabled) {
        m_enableJavascript = enabled;
        view->page()->settings()->setAttribute(QWebSettings::JavascriptEnabled, enabled);
    }

    void toggleEnableJava(bool enabled) {
        m_enableJava = enabled;
        view->page()->settings()->setAttribute(QWebSettings::JavaEnabled, enabled);
    }

    void toggleEnableImages(bool enabled) {
        m_enableImages = enabled;
        view->page()->settings()->setAttribute(QWebSettings::AutoLoadImages, enabled);
    }

    void toggleEnablePlugins(bool enabled) {
        m_enablePlugins = enabled;
        view->page()->settings()->setAttribute(QWebSettings::PluginsEnabled, enabled);
    }

    void saveHunterConfig() {
        m_hunterEnabled = hunterConfig->hunterEnabled();
        m_hunterPath = hunterConfig->progPath();
        m_vdomPath   = hunterConfig->vdomPath();
    }

    void execHunterConfig() {
        initHunterConfig();
        hunterConfig->exec();
    }

private:

    void initHunterConfig() {
        hunterConfig->setHunterEnabled(m_hunterEnabled);
        hunterConfig->setProgPath(m_hunterPath);
        hunterConfig->setVdomPath(m_vdomPath);
    }

    QVector<int> zoomLevels;
    int currentZoom;

    // create the status bar, tool bar & menu
    void setupUI();

    void createCentralWidget();
    void createWebView();
    void createSideBar();
    void createToolBar();

    void createMenus();
    void createFileMenu();
    void createEditMenu();
    void createViewMenu();
    void createSettingsMenu();
    void createHelpMenu();

    void createProgressBar();
    void createUrlEdit();

    void writeSettings();
    void readSettings();

    QTextEdit* itemInfoEdit;
    QTextEdit* pageInfoEdit;

    QWebView *view;
    LineEdit *urlEdit;
    QWidget *sidebar;
    QProgressBar *progress;
    HunterConfigDialog* hunterConfig;

    QStringList urlList;
    QStringListModel urlModel;
    QSettings* settings;

    bool m_enableJavascript;
    bool m_enablePlugins;
    bool m_enableImages;
    bool m_enableJava;

    bool m_hunterEnabled;
    QString m_hunterPath;
    QString m_vdomPath;
};

#endif

