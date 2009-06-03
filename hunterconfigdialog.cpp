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

    QPushButton* button = new QPushButton(tr("Browse..."), this);
    connect(button, SIGNAL(clicked()),
            this, SLOT(browseProgFile()));
    formLayout->addWidget(button, 0, 2);

    label = new QLabel(tr("&VDOM output path"), this);
    formLayout->addWidget(label, 1, 0);

    vdomPathEdit = new QLineEdit(this);
    formLayout->addWidget(vdomPathEdit, 1, 1);
    label->setBuddy(vdomPathEdit);

    button = new QPushButton(tr("Browse..."), this);
    connect(button, SIGNAL(clicked()),
            this, SLOT(browseVdomFile()));
    formLayout->addWidget(button, 1, 2);

    formLayout->setSpacing(20);

    layout->addWidget(formGroup);

    QHBoxLayout* buttonsLayout = new QHBoxLayout;
    buttonsLayout->addSpacing(300);

    button = new QPushButton(tr("&Save"), this);
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

void HunterConfigDialog::accept() {
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

void HunterConfigDialog::browseProgFile() {
     const QString& fileName = QFileDialog::getOpenFileName(
         this, tr("Hunter Program File"),
         0, tr("Executable files (*.bat *.pl *.sh *.exe);;Any Files (*)"));
     if (!fileName.isEmpty()) {
         progPathEdit->setText(fileName);
     }
}

void HunterConfigDialog::browseVdomFile() {
     const QString& fileName = QFileDialog::getSaveFileName(
         this, tr("VDOM Output File"),
         0, tr("Text Files (*.txt *.dom *.vdom)"));
     if (!fileName.isEmpty()) {
         vdomPathEdit->setText(fileName);
     }
}

