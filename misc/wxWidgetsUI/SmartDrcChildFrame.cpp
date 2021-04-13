
#include "pch.h"

#include "SmartDrcChildFrame.h"

struct cSmartrcChildFrame::cPersist : public wxPersistentTLW
{
   wxString m_perspective;

   using wxPersistentTLW::wxPersistentTLW;

   void Save() const wxOVERRIDE
   {
      auto pFrame = (cSmartrcChildFrame *)Get();
      SaveField("perspective", m_perspective);
   }
   bool Restore() wxOVERRIDE
   {
      auto pFrame = (cSmartrcChildFrame *)Get();
      RestoreField("perspective", m_perspective);
      pFrame->m_dockmgr.LoadPerspective(m_perspective);
      return true;
   }
   bool SaveField(const wxString &name, const wxString &value) const
   {
      auto &pmgr = wxPersistenceManager::Get();
      return pmgr.SaveValue(*this, name, value);
   }
   bool RestoreField(const wxString &name, wxString &value)
   {
      auto &pmgr = wxPersistenceManager::Get();
      return pmgr.RestoreValue(*this, name, &value);
   }
};

cSmartrcChildFrame::cSmartrcChildFrame(wxDocument *doc, wxView *view, wxAuiMDIParentFrame *parent, wxWindowID id, const wxString &title,
   const wxPoint &pos, const wxSize &size, long style, const wxString &name)
   : cFrameAny(doc, view, parent, id, title, pos, size, style, name)
{
   m_dockmgr.SetManagedWindow(this);
}

cSmartrcChildFrame::~cSmartrcChildFrame()
{
   if (m_persist) {
      m_persist->m_perspective = m_dockmgr.SavePerspective();
   }
}

wxBEGIN_EVENT_TABLE(cSmartrcChildFrame, cFrameAny) wxEND_EVENT_TABLE()

void cSmartrcChildFrame::Init(cPcbDesignDocument *pDoc)
{
   m_project_explorer.reset(new cSmartDrcProjectExplorer(pDoc, this, 1));
   m_dockmgr.AddPane(&*m_project_explorer, wxAuiPaneInfo().Name("Project Explorer").LeftDockable());

   m_draw_area.reset(new cDrawArea(this, 2, pDoc));
   m_dockmgr.AddPane(&*m_draw_area, wxAuiPaneInfo().Name("BoardViewer").CenterPane().PaneBorder(false));

   m_dockmgr.Update();

   auto fpath = pDoc->GetFilename();
   auto fname = "ChildFrame(" + fs::path(fpath).filename().string() + ")";
   wxPersistentRegisterAndRestore(this, fname.c_str());

   m_draw_area->Init();
   m_project_explorer->Init();
}

wxPersistentObject *wxCreatePersistentObject(cSmartrcChildFrame *obj)
{
   obj->m_persist = new cSmartrcChildFrame::cPersist(obj);
   return obj->m_persist;
}
