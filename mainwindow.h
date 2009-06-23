#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/* from QJson: http://qjson.sourceforge.net/docs/index.html */
#include <qjson/json_driver.hh>
#include <qwebvdom.h> /* added to WebCore by Yahoo! China EEEE */
#include <qwebview.h>
#include <qwebframe.h>
#include <qwebsettings.h>
#include <QtGui>

#include "lineedit.h"
#include "aboutdialog.h"
#include "hunterconfigdialog.h"
#include "iteratorconfigdialog.h"
#include "iterator.h"

#include <qwebselected.h>
#include "fielddialog.h"
#include "viwiedialog.h"
#include "webview.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(const QString& url = QString());

    QWebPage* webPage() const {
        return m_view->page();
    }

    QWebView* webView() const {
        return m_view;
    }

protected:

    virtual void closeEvent(QCloseEvent * event);

protected slots:

    void iterPrev();
    void iterNext();

    void huntOnly();

    void hunterStarted() {
        statusBar()->showMessage("Hunter " + m_hunterPath + " started.");
    }

    void hunterFinished(int exitCode, QProcess::ExitStatus);

    void emitHunterStdout() {
        m_itemInfoEdit->append(QString::fromUtf8(m_hunter.readAllStandardOutput()));
    }

    void emitHunterStderr() {
        m_itemInfoEdit->append(QString::fromUtf8(m_hunter.readAllStandardError()));
    }

    void loadUrl(const QUrl& url);

    // viwie slots
    void showTextFieldDialog();
    void showImageFieldDialog();
    void showViwieDialog();
    void hideViwieDialog();

    void addField();
    void overrideField();
    void deleteField();

    void tmpSaveViwie();
    void permanentSaveViwie();
    void showDetailField(const QVariant& detail);

    void updateUrl(const QUrl& url) {
        m_urlEdit->setText(url.toEncoded());
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
        m_urlEdit->selectAll();
        m_urlEdit->setFocus();
    }

    void changeLocation();

    void loadFinished(bool done);

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

        m_view->setZoomFactor(qreal(currentZoom)/100.0);
    }

    void zoomOut() {
        int i = zoomLevels.indexOf(currentZoom);
        Q_ASSERT(i >= 0);
        if (i > 0)
            currentZoom = zoomLevels[i - 1];

        m_view->setZoomFactor(qreal(currentZoom)/100.0);
    }

    void resetZoom()
    {
       currentZoom = 100;
       m_view->setZoomFactor(1.0);
    }

    void toggleZoomTextOnly(bool b)
    {
        m_view->page()->settings()->setAttribute(QWebSettings::ZoomTextOnly, b);
    }

    void print() {
#if QT_VERSION >= 0x040400 && !defined(QT_NO_PRINTER)
        QPrintPreviewDialog dlg(this);
        connect(&dlg, SIGNAL(paintRequested(QPrinter *)),
                m_view, SLOT(print(QPrinter *)));
        dlg.exec();
#endif
    }

    void aboutMe() {
        AboutDialog* about = new AboutDialog(this);
        about->show();
    }

    void toggleEnableJavascript(bool enabled) {
        m_enableJavascript = enabled;
        m_view->page()->settings()->setAttribute(QWebSettings::JavascriptEnabled, enabled);
    }

    void toggleEnableJava(bool enabled) {
        m_enableJava = enabled;
        m_view->page()->settings()->setAttribute(QWebSettings::JavaEnabled, enabled);
    }

    void toggleEnableImages(bool enabled) {
        m_enableImages = enabled;
        m_view->page()->settings()->setAttribute(QWebSettings::AutoLoadImages, enabled);
    }

    void toggleEnablePlugins(bool enabled) {
        m_enablePlugins = enabled;
        m_view->page()->settings()->setAttribute(QWebSettings::PluginsEnabled, enabled);
    }

    void saveHunterConfig();
    void saveIteratorConfig();

    void execHunterConfig();
    void execIteratorConfig();

private:

    void initIterator();

    QVariant evalJS(const QString& js);

    void addUrlToList();
    void initHunterConfig();

    void initIteratorConfig();

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

    void annotateWebPage(QVariantList& groups);

    QTextEdit* m_itemInfoEdit;
    QTextEdit* m_pageInfoEdit;

    WebView *m_view;
    LineEdit *m_urlEdit;
    QSplitter *m_sidebar;
    QProgressBar *m_progress;
    HunterConfigDialog* m_hunterConfig;
    QLabel *m_hunterLabel;

    IteratorConfigDialog* m_iteratorConfig;

    bool m_viwieEnabled;
    void createViwieUI();
    QWebSelected* m_webSelected;
    ViwieDialog *m_viwieDialog;
    FieldDialog *m_fieldDialog;
    QVariant viwieTags;

    QStringList m_urlList;
    QStringListModel m_urlCompleterModel;
    QSettings* m_settings;

    bool m_enableJavascript;
    bool m_enablePlugins;
    bool m_enableImages;
    bool m_enableJava;

    bool m_hunterEnabled;
    QString m_hunterPath;
    QString m_vdomPath;

    bool m_iteratorEnabled;
    QString m_urlListFile;

    QWebVDom* m_webvdom;
    QProcess m_hunter;
    QPushButton* m_huntButton;

    QPushButton* m_iterPrevButton;
    QPushButton* m_iterNextButton;

    QLabel* m_iterLabel;

    JSonDriver m_jsonDriver;

    Iterator m_iterator;

    QSplitter* m_mainSplitter;
};

#endif

