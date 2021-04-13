/********************************************************************************
** Form generated from reading the UI file 'CopperSpiceUI.ui'
**
** Created by: CopperSpice User Interface Compiler, Version 1.6.3
**
** WARNING! Any changes made to this file will be lost when the UI file is recompiled
********************************************************************************/

#ifndef UI_COPPERSPICEUI_H
#define UI_COPPERSPICEUI_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CopperSpiceUIClass
{
public:
    QAction *actionOpen;
    QWidget *centralWidget;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *CopperSpiceUIClass)
    {
        if (CopperSpiceUIClass->objectName().isEmpty())
            CopperSpiceUIClass->setObjectName(QString::fromUtf8("CopperSpiceUIClass"));
        CopperSpiceUIClass->resize(600, 400);
        actionOpen = new QAction(CopperSpiceUIClass);
        actionOpen->setObjectName(QString::fromUtf8("actionOpen"));
        centralWidget = new QWidget(CopperSpiceUIClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        CopperSpiceUIClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(CopperSpiceUIClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 600, 21));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        CopperSpiceUIClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(CopperSpiceUIClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        CopperSpiceUIClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(CopperSpiceUIClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        CopperSpiceUIClass->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuFile->addAction(actionOpen);

        retranslateUi(CopperSpiceUIClass);

        QMetaObject::connectSlotsByName(CopperSpiceUIClass);
    } // setupUi

    void retranslateUi(QMainWindow *CopperSpiceUIClass)
    {
        CopperSpiceUIClass->setWindowTitle(QApplication::translate("CopperSpiceUIClass", "CopperSpiceUI", 0, QApplication::UnicodeUTF8));
        actionOpen->setText(QApplication::translate("CopperSpiceUIClass", "Open", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("CopperSpiceUIClass", "File", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class CopperSpiceUIClass: public Ui_CopperSpiceUIClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COPPERSPICEUI_H
