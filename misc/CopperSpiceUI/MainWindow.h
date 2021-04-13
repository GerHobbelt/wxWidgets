#pragma once
#include <qmainwindow.h>
#include <qmdiarea.h>
#include "mdichild.h"
#include "ui_CopperSpiceUI.h"

class cMainWindow : public QMainWindow
{
   CS_OBJECT(cMainWindow)

   QMdiSubWindow *findMdiChild(const QString &fileName) const;
   bool loadFile(const QString &fileName);
   cMdiChild *createMdiChild();
   void prependToRecentFiles(const QString& fileName);

public:
   explicit cMainWindow();

   bool openFile(const QString &fileName);

private:
   Ui::CopperSpiceUIClass *m_ui;
   QMdiArea *m_mdiArea;

   CS_SLOT_1(Private, void on_actionOpen_triggered());
   CS_SLOT_2(on_actionOpen_triggered);
};
