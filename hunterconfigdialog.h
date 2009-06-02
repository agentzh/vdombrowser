#ifndef HUNTER_CONFIG_DIALOG_H
#define HUNTER_CONFIG_DIALOG_H

#include <qdialog.h>
#include <QtGui>

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

private:
    QLineEdit* progPathEdit;
    QLineEdit* vdomPathEdit;
    QGroupBox* formGroup;
};

#endif // HUNTER_CONFIG_DIALOG_H

