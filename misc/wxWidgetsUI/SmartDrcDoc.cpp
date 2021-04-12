
#include "pch.h"

#include "SmartDrcDoc.h"
#include "smartdrc.h"

bool cPcbDesignDocument::OnCreate(const wxString& fpath, long flags)
{
   if (!wxDocument::OnCreate(fpath, flags)) {
      return false;
   }

   std::filesystem::path path = fpath.c_str().AsInternal();
   m_db = load_design(path);

   return true;
}

wxIMPLEMENT_DYNAMIC_CLASS(cPcbDesignDocument, wxDocument);
