#ifndef JSAPPBRIDGE_H
#define JSAPPBRIDGE_H

#include <QtCore>
#include "mainwindow.h"

class JSAppBridge: public QObject {
public:
    JSAppBridge(MainWindow* win): m_win(win) {}

public slots:
    void setItemInfo(const QString& msg) {
        m_win->itemInfoEdit()->setText(msg);
    }

    void setPageInfo(const QString& msg) {
        m_win->pageInfoEdit()->setText(msg);
    }

    void setStatusInfo(const QString& msg) {
        m_win->statusBar()->showMessage(msg);
    }

private:
    MainWindow* m_win;
};

#endif // JSAPPBRIDGE_H

