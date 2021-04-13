
#include "pch.h"
#include <QMessageBox.h>
#include <QCloseEvent.h>
#include "mdichild.h"

cMdiChild::cMdiChild()
{
   setAttribute(Qt::WA_DeleteOnClose);
   isUntitled = true;
}

bool cMdiChild::loadFile(const QString &fileName)
{
   QFile file(fileName);
   if (!file.open(QFile::ReadOnly | QFile::Text)) {
      QMessageBox::warning(this, tr("MDI"), "Cannot read file " + fileName + ":\n" + file.errorString() + ".");
      return false;
   }

   QGuiApplication::setOverrideCursor(Qt::WaitCursor);
   m_db.reset(load_design(fileName.data()));
   QGuiApplication::restoreOverrideCursor();

   setCurrentFile(fileName);

   return true;
}

void cMdiChild::closeEvent(QCloseEvent *event)
{
   if (maybeSave()) {
      event->accept();
   }
   else {
      event->ignore();
   }
}

bool cMdiChild::maybeSave()
{
   //if (!document()->isModified()) {
   //   return true;
   //}
   //const QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("MDI"),
   //   "'" + userFriendlyCurrentFile() + "' has been modified.\n"
   //   "Do you want to save your changes?",
   //   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
   //switch (ret) {
   //   case QMessageBox::Save:
   //      return save();
   //   case QMessageBox::Cancel:
   //      return false;
   //   default:
   //      break;
   //}
   return true;
}

void cMdiChild::setCurrentFile(const QString &fileName)
{
   curFile = QFileInfo(fileName).canonicalFilePath();
   isUntitled = false;
   setWindowModified(false);
   setWindowTitle(userFriendlyCurrentFile() + "[*]");
}

QString cMdiChild::userFriendlyCurrentFile()
{
   return strippedName(curFile);
}

QString cMdiChild::strippedName(const QString &fullFileName)
{
   return QFileInfo(fullFileName).fileName();
}

void cMdiChild::paintEvent(QPaintEvent* event)
{
   QWidget::paintEvent(event);
}
