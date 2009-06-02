#include "hunterconfigdialog.h"

HunterConfigDialog::HunterConfigDialog(QWidget *parent): QDialog(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    formGroup = new QGroupBox(tr("&Enable X Hunter"), this);
    formGroup->setCheckable(true);
    formGroup->setChecked(false);

    QGridLayout* formLayout = new QGridLayout(formGroup);
    formGroup->setLayout(formLayout);
    //formGroup->setFlat(false);

    QLabel *label = new QLabel(tr("&Hunter program path"), this);
    formLayout->addWidget(label, 0, 0);

    progPathEdit = new QLineEdit(this);
    formLayout->addWidget(progPathEdit, 0, 1);
    label->setBuddy(progPathEdit);

    label = new QLabel(tr("&VDOM output path"), this);
    formLayout->addWidget(label, 1, 0);

    vdomPathEdit = new QLineEdit(this);
    formLayout->addWidget(vdomPathEdit, 1, 1);
    label->setBuddy(vdomPathEdit);

    formLayout->setSpacing(20);

    layout->addWidget(formGroup);

    QHBoxLayout* buttonsLayout = new QHBoxLayout;
    buttonsLayout->addSpacing(300);

    QPushButton* button = new QPushButton(tr("&Save"), this);
    connect(button, SIGNAL(clicked()),
            this, SLOT(accept()));
    buttonsLayout->addWidget(button);

    button = new QPushButton(tr("&Cancel"), this);
    connect(button, SIGNAL(clicked()),
            this, SLOT(reject()));
    buttonsLayout->addWidget(button);
    buttonsLayout->addStretch();

    layout->addLayout(buttonsLayout);
    //layout->addStretch();

    setLayout(layout);
    setFixedSize(QSize(550, 200));
    setWindowTitle(tr("X Hunter Configuration"));
}

