
#include "pch.h"
#include <QMessageBox.h>
#include <QCloseEvent.h>
#include "mdichild.h"
#include "cs_utils.h"

cMdiChild::cMdiChild()
{
   setAttribute(Qt::WA_DeleteOnClose);
   isUntitled = true;
}

bool cMdiChild::loadFile(const QString &fileName)
{
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

void cMdiChild::resizeEvent(QResizeEvent *event)
{
   UpdateScrollBars(false);
   auto size = event->size();
   cScreenRect rc = csToScreen(QRect({0, 0}, size));
   m_conv.SetScreen(rc);
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
   m_cvd.reset(new cOptionsImp(fileName.data()));
   OnRestoreView();
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

   if (!m_db) {
      return;
   }

   using namespace std::chrono;
   auto time_start = steady_clock::now();

   QRect rcDraw = event->rect();

   QPainter painter(this);
   QImage offbmp = Render(m_db->database(), rcDraw);
   painter.drawImage(QPoint(0, 0), offbmp);

   auto time_finish = steady_clock::now();
   auto out_time = [this](const char *msg, auto time) {
      std::stringstream ss;
      ss << msg << duration_cast<milliseconds>(time).count();
      ss << "ms" << std::endl;
      //if (auto frame = (cSmartDrcFrame *)wxGetApp().GetTopWindow()) {
      //   frame->SetStatusText(cSmartDrcFrame::Field_Elapsed, ss.str().c_str());
      //}
   };
   out_time("Elapsed: ", time_finish - time_start);
}

QImage cMdiChild::Render(cDatabase* pDB, const QRect& rc) const
{
   QDib retval;
   if (m_cvd) {
      auto sz = rc.size();
      if (int size = sz.width() * sz.height()) {
         LOG("    Rendering {0}:{1}:{2}:{3}", rc.left(), rc.bottom(), rc.width(), rc.height());

         retval.resize(sz.width(), sz.height());

         auto pOpt = m_cvd.get();
         auto conv = m_conv.Rebind(csToScreen(rc));

         DrawBL2D(pDB, &retval, conv, pOpt);

         LOG("    Rendering completed");
      }
   }
   return retval.m_image;
}

void cMdiChild::OnRestoreView()
{
   if (m_db) {
      auto db = m_db->database();
      cScreenRect rc(csToScreen(this->rect()));
      std::filesystem::path fname = curFile.data();
      cDrawAreaBase::OnRestoreView(db, rc, fname);
   }
}

void cMdiChild::UpdateScrollBars(bool bRedraw)
{
   //TBD
}
