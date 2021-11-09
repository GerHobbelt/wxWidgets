/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xrc/xh_aui.h
// Purpose:     XRC resource handler for wxAUI
// Author:      Andrea Zanellato, Steve Lamerton (wxAuiNotebook)
// Created:     2011-09-18
// Copyright:   (c) 2011 wxWidgets Team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_XH_AUI_H_
#define _WX_XH_AUI_H_

#include "wx/xrc/xmlres.h"

#if wxUSE_XRC && wxUSE_AUI

import <vector>;

class wxAuiManager;
class wxAuiNotebook;

class wxAuiXmlHandler : public wxXmlResourceHandler
{
public:
    wxAuiXmlHandler();
    wxObject *DoCreateResource() override;
    bool CanHandle(wxXmlNode *node) override;

    // Returns the wxAuiManager for the specified window
    wxAuiManager *GetAuiManager(wxWindow *managed) const;

private:
    // Used to UnInit() the wxAuiManager before destroying its managed window
    void OnManagedWindowClose(wxWindowDestroyEvent &event);

    using Managers = std::vector<wxAuiManager *>;
    Managers m_managers; // all wxAuiManagers created in this handler

    wxAuiManager    *m_manager{nullptr};  // Current wxAuiManager
    wxWindow        *m_window{nullptr};   // Current managed wxWindow
    wxAuiNotebook   *m_notebook{nullptr};

    bool m_mgrInside{false}; // Are we handling a wxAuiManager or panes inside it?
    bool m_anbInside{false}; // Are we handling a wxAuiNotebook or pages inside it?

    wxDECLARE_DYNAMIC_CLASS(wxAuiXmlHandler);
};

#endif //wxUSE_XRC && wxUSE_AUI

#endif //_WX_XH_AUI_H_
