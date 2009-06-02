#include "webpage.h"
#include "mainwindow.h"
#include <QtUiTools/QUiLoader>

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
