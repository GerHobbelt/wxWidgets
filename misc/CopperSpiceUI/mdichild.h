#pragma once

#include "smartdrc.h"
#include "options_imp.h"

#include "../smartdrc/DrawAreaBase.h"

class cMdiChild
   : public QWidget
   , public cDrawAreaBase
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
   void resizeEvent(QResizeEvent *event) override;

private:
   bool maybeSave();
   void setCurrentFile(const QString &fileName);
   QString strippedName(const QString &fullFileName);

   void OnRestoreView();
   QImage Render(cDatabase* pDB, const QRect& rc) const;
   void UpdateScrollBars(bool bRedraw = true) override;

   QString curFile;
   bool isUntitled;
/*
   void OnRestoreView();
   void UpdateScrollBars(bool bRedraw = true);
   void UpdateAfterScroll(const cScreenUpdateDesc screen_update_data);
   void OnHScroll(wxEventType evType, UINT nPos);
   void OnVScroll(wxEventType evType, UINT nPos);

   void OnDraw(wxPaintEvent &evt);
   void OnEraseBkgnd(wxEraseEvent& evt);
   void OnSize(wxSizeEvent &evt);
   void OnMouseMove(wxMouseEvent&);
   void OnMouseWheel(wxMouseEvent&);
   bool ProcessEvent(wxEvent &event) override;

   wxBitmap Render(cDatabase *pDB, const wxRect &rc) const;

public:
   cPcbDesignDocument *m_document;

   cDrawArea(wxWindow* parent, wxWindowID id, cPcbDesignDocument* doc);
   ~cDrawArea();

   void Init();

private:
   wxDECLARE_EVENT_TABLE();
*/
};
