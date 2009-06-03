#ifndef HUNTER_CONFIG_DIALOG_H
#define HUNTER_CONFIG_DIALOG_H

#include <qdialog.h>
#include <QtGui>
//#include <QDebug>

class HunterConfigDialog: public QDialog {
    Q_OBJECT

public:
    HunterConfigDialog(QWidget *parent = 0);

    void setHunterEnabled(bool enabled) {
        formGroup->setChecked(enabled);
    }

    void setProgPath(const QString& path) {
        progPathEdit->setText(path);
    }

    void setVdomPath(const QString& path) {
        vdomPathEdit->setText(path);
    }

    bool hunterEnabled() {
        return formGroup->isChecked();
    }

    QString progPath() const {
        return progPathEdit->text();
    }

    QString vdomPath() const {
        return vdomPathEdit->text();
    }

public slots:
    virtual void accept();
    void browseProgFile();
    void browseVdomFile();

private:
    void croak(const QString& msg) {
        QMessageBox::warning(this, tr("X Hunter Configuration"),
            msg, QMessageBox::NoButton);
    }
    QLineEdit* progPathEdit;
    QLineEdit* vdomPathEdit;
    QGroupBox* formGroup;
};

#endif // HUNTER_CONFIG_DIALOG_H

