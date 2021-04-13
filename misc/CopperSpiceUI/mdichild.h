#pragma once
#include <qtextedit.h>
#include "smartdrc.h"

class cMdiChild : public QWidget
{
   CS_OBJECT(cMdiChild)

public:
   cMdiChild();

   void newFile();
   bool loadFile(const QString &fileName);
   bool save();
   bool saveAs();
   bool saveFile(const QString &fileName);
   QString userFriendlyCurrentFile();
   QString currentFile()
   {
      return curFile;
   }

   void paintEvent(QPaintEvent *event) override;

protected:
   std::unique_ptr<iDbHolder> m_db;

   void closeEvent(QCloseEvent *event) override;

private:
   bool maybeSave();
   void setCurrentFile(const QString &fileName);
   QString strippedName(const QString &fullFileName);

   QString curFile;
   bool isUntitled;
};
