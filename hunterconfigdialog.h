#ifndef HUNTER_CONFIG_DIALOG_H
#define HUNTER_CONFIG_DIALOG_H

#include <qdialog.h>
#include <QtGui>

class HunterConfigDialog: public QDialog {
    Q_OBJECT

public:
    HunterConfigDialog(QWidget *parent = 0);

    QLineEdit* progPathEdit;
    QLineEdit* vdomPathEdit;
};

#endif // HUNTER_CONFIG_DIALOG_H

