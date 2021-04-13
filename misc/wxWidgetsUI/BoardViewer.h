
#pragma once

#include "wx/docview.h"
#include "wx/scrolbar.h"

#include "options_imp.h"
#include "screen_coord_converter.h"

#include "SmartDrcDoc.h"
#include "ProjectExplorer.h"

using cDrawAreaBase = wxWindow//wxScrolledWindow
;

class cDrawArea : public cDrawAreaBase
{
   using coord_t = cCoordConverter::coord_t;
   using cScreenRect = cCoordConverter::cScreenRect;
   using cScreenPoint = cCoordConverter::cScreenPoint;
   using cScreenUpdateDesc = cCoordConverter::cScreenUpdateDesc;

   cCoordConverter m_conv;
   cScreenPoint m_scroll_size;
   std::unique_ptr<cOptionsImp> m_cvd;

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
};

class cSmartDrcBoardView : public wxView
{
public:
   cSmartDrcBoardView();
   ~cSmartDrcBoardView();
   void OnDraw(wxDC* dc) override {}
   bool OnCreate(wxDocument* doc, long flags) override;
   void OnUpdate(wxView* sender, wxObject* hint = NULL) override;

private:
   wxDECLARE_DYNAMIC_CLASS(cSmartDrcBoardView);
};
