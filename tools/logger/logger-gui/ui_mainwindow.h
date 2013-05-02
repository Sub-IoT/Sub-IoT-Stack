/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Thu 2. May 08:41:48 2013
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
#include <QtGui/QSplitter>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include <qcustomplot/qcustomplot.h>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *connectAction;
    QAction *restartAction;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QSplitter *splitter;
    QCustomPlot *plotWidget;
    QPlainTextEdit *parsedOutputPlainTextEdit;
    QMenuBar *menubar;
    QStatusBar *statusbar;
    QToolBar *toolBar;
    QToolBar *toolBar_2;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(800, 600);
        connectAction = new QAction(MainWindow);
        connectAction->setObjectName(QString::fromUtf8("connectAction"));
        connectAction->setCheckable(true);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/resources/network-wired.png"), QSize(), QIcon::Normal, QIcon::Off);
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
        splitter = new QSplitter(centralwidget);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setOrientation(Qt::Vertical);
        plotWidget = new QCustomPlot(splitter);
        plotWidget->setObjectName(QString::fromUtf8("plotWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(plotWidget->sizePolicy().hasHeightForWidth());
        plotWidget->setSizePolicy(sizePolicy);
        plotWidget->setMinimumSize(QSize(0, 250));
        splitter->addWidget(plotWidget);
        parsedOutputPlainTextEdit = new QPlainTextEdit(splitter);
        parsedOutputPlainTextEdit->setObjectName(QString::fromUtf8("parsedOutputPlainTextEdit"));
        QFont font;
        font.setFamily(QString::fromUtf8("Monospace"));
        font.setPointSize(10);
        parsedOutputPlainTextEdit->setFont(font);
        splitter->addWidget(parsedOutputPlainTextEdit);

        verticalLayout->addWidget(splitter);

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
        toolBar_2 = new QToolBar(MainWindow);
        toolBar_2->setObjectName(QString::fromUtf8("toolBar_2"));
        MainWindow->addToolBar(Qt::TopToolBarArea, toolBar_2);

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
        toolBar_2->setWindowTitle(QApplication::translate("MainWindow", "toolBar_2", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
