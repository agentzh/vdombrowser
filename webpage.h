#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <qwebpage.h>

class WebPage : public QWebPage
{
public:
    WebPage(QWidget *parent) : QWebPage(parent) {}

    virtual QWebPage *createWindow(QWebPage::WebWindowType);
    virtual QObject* createPlugin(const QString&, const QUrl&, const QStringList&, const QStringList&);
};

#endif

