
#include "pch.h"

#include "ProjectExplorer.h"

cSmartDrcProjectExplorer::cSmartDrcProjectExplorer(cPcbDesignDocument* doc, wxWindow* parent, wxWindowID id,
   const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator)
   : wxDataViewTreeCtrl(parent, id, pos, size, style, validator)
   , m_doc(doc)
{
}

void cSmartDrcProjectExplorer::Init()
{
   auto pModel = new wxDataViewTreeStore;
   AssociateModel(pModel);
   pModel->DecRef();

   auto root = pModel->GetRoot()->GetItem();

   auto layers = pModel->AppendContainer(root, L"Layers");
   pModel->ItemAdded(root, layers);

   auto pDB = m_doc->database();
   for (auto& it : pDB->Layers()) {
      auto layer = pModel->AppendItem(layers, it.getName());
   }
}
