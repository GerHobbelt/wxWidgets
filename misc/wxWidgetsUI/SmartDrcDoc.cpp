
#include "pch.h"

#include "SmartDrcDoc.h"
#include "smartdrc.h"

bool cPcbDesignDocument::OnOpenDocument(const wxString& fpath)
{
   std::filesystem::path path = fpath.c_str().AsInternal();
   m_db.reset(load_design(path));

   return wxDocument::OnOpenDocument(fpath);
}

wxIMPLEMENT_DYNAMIC_CLASS(cPcbDesignDocument, wxDocument);
