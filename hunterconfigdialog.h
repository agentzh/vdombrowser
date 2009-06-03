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

    bool hunterEnabled() const {
        return formGroup->isChecked();
    }

    QString progPath() const {
        return progPathEdit->text();
    }

    QString vdomPath() const {
        return vdomPathEdit->text();
    }

public slots:
    virtual void accept() {
        //qDebug() << "Checking form values...\n";
        if (formGroup->isChecked()) {
            if (progPathEdit->text().isEmpty()) {
                croak(tr("Hunter Program Path is empty."));
                return;
            }
            if (vdomPathEdit->text().isEmpty()) {
                croak(tr("VDOM Output File Path is empty."));
                return;
            }
        }
        QDialog::accept();
    }

private:
    void croak(const QString& msg) {
        QMessageBox::warning(this, tr("X Hunter Configuration Error"),
            msg, QMessageBox::NoButton);
    }
    QLineEdit* progPathEdit;
    QLineEdit* vdomPathEdit;
    QGroupBox* formGroup;
};

#endif // HUNTER_CONFIG_DIALOG_H

