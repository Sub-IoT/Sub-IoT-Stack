/********************************************************************************
** Form generated from reading UI file 'connectdialog.ui'
**
** Created: Wed 27. Feb 16:25:25 2013
**      by: Qt User Interface Compiler version 4.8.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONNECTDIALOG_H
#define UI_CONNECTDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QStackedWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ConnectDialog
{
public:
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QComboBox *connectionTypeComboBox;
    QStackedWidget *stackedWidget;
    QWidget *serialPortSettings;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QComboBox *serialPortComboBox;
    QWidget *fileSettings;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_3;
    QLineEdit *fileNameLineEdit;
    QPushButton *selectFileButton;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ConnectDialog)
    {
        if (ConnectDialog->objectName().isEmpty())
            ConnectDialog->setObjectName(QString::fromUtf8("ConnectDialog"));
        ConnectDialog->resize(400, 300);
        verticalLayout_2 = new QVBoxLayout(ConnectDialog);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(ConnectDialog);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        connectionTypeComboBox = new QComboBox(ConnectDialog);
        connectionTypeComboBox->setObjectName(QString::fromUtf8("connectionTypeComboBox"));

        horizontalLayout->addWidget(connectionTypeComboBox);


        verticalLayout_2->addLayout(horizontalLayout);

        stackedWidget = new QStackedWidget(ConnectDialog);
        stackedWidget->setObjectName(QString::fromUtf8("stackedWidget"));
        serialPortSettings = new QWidget();
        serialPortSettings->setObjectName(QString::fromUtf8("serialPortSettings"));
        verticalLayout = new QVBoxLayout(serialPortSettings);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_2 = new QLabel(serialPortSettings);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_2->addWidget(label_2);

        serialPortComboBox = new QComboBox(serialPortSettings);
        serialPortComboBox->setObjectName(QString::fromUtf8("serialPortComboBox"));

        horizontalLayout_2->addWidget(serialPortComboBox);


        verticalLayout->addLayout(horizontalLayout_2);

        stackedWidget->addWidget(serialPortSettings);
        fileSettings = new QWidget();
        fileSettings->setObjectName(QString::fromUtf8("fileSettings"));
        verticalLayout_3 = new QVBoxLayout(fileSettings);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_3 = new QLabel(fileSettings);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_3->addWidget(label_3);

        fileNameLineEdit = new QLineEdit(fileSettings);
        fileNameLineEdit->setObjectName(QString::fromUtf8("fileNameLineEdit"));

        horizontalLayout_3->addWidget(fileNameLineEdit);

        selectFileButton = new QPushButton(fileSettings);
        selectFileButton->setObjectName(QString::fromUtf8("selectFileButton"));

        horizontalLayout_3->addWidget(selectFileButton);


        verticalLayout_3->addLayout(horizontalLayout_3);

        stackedWidget->addWidget(fileSettings);

        verticalLayout_2->addWidget(stackedWidget);

        buttonBox = new QDialogButtonBox(ConnectDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout_2->addWidget(buttonBox);


        retranslateUi(ConnectDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), ConnectDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), ConnectDialog, SLOT(reject()));

        stackedWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(ConnectDialog);
    } // setupUi

    void retranslateUi(QDialog *ConnectDialog)
    {
        ConnectDialog->setWindowTitle(QApplication::translate("ConnectDialog", "Dialog", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("ConnectDialog", "Connection type", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("ConnectDialog", "Serial port", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("ConnectDialog", "Capture file", 0, QApplication::UnicodeUTF8));
        selectFileButton->setText(QApplication::translate("ConnectDialog", " ...", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ConnectDialog: public Ui_ConnectDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONNECTDIALOG_H
