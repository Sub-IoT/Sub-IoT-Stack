/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Wed 19. Dec 15:35:38 2012
**      by: Qt User Interface Compiler version 4.8.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *connectAction;
    QAction *restartAction;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QPlainTextEdit *parsedOutputPlainTextEdit;
    QMenuBar *menubar;
    QStatusBar *statusbar;
    QToolBar *toolBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(800, 600);
        connectAction = new QAction(MainWindow);
        connectAction->setObjectName(QString::fromUtf8("connectAction"));
        connectAction->setCheckable(true);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/resources/media-record.png"), QSize(), QIcon::Normal, QIcon::Off);
        connectAction->setIcon(icon);
        restartAction = new QAction(MainWindow);
        restartAction->setObjectName(QString::fromUtf8("restartAction"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/resources/system-reboot.png"), QSize(), QIcon::Normal, QIcon::Off);
        restartAction->setIcon(icon1);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        parsedOutputPlainTextEdit = new QPlainTextEdit(centralwidget);
        parsedOutputPlainTextEdit->setObjectName(QString::fromUtf8("parsedOutputPlainTextEdit"));
        QFont font;
        font.setFamily(QString::fromUtf8("Monospace"));
        font.setPointSize(10);
        parsedOutputPlainTextEdit->setFont(font);

        verticalLayout->addWidget(parsedOutputPlainTextEdit);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 25));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);
        toolBar = new QToolBar(MainWindow);
        toolBar->setObjectName(QString::fromUtf8("toolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, toolBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
        connectAction->setText(QApplication::translate("MainWindow", "Connect", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        connectAction->setToolTip(QApplication::translate("MainWindow", "Connect", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        restartAction->setText(QApplication::translate("MainWindow", "Restart", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        restartAction->setToolTip(QApplication::translate("MainWindow", "Restart", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        toolBar->setWindowTitle(QApplication::translate("MainWindow", "toolBar", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
