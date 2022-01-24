#ifndef _WX_DOCKING_DEFS_H_
#define _WX_DOCKING_DEFS_H_

#include "wx/defs.h"

#define STRING2(x) #x  
#define STRING(x) STRING2(x)  
#define WARNING(txt) message(__FILE__ "(" STRING(__LINE__)"): warning " txt)
// Usage: #pragma WARNING(message)

class WXDLLIMPEXP_CORE wxWindow;
class WXDLLIMPEXP_CORE wxDockingFrame;
class WXDLLIMPEXP_CORE wxSplitterWindow;
class WXDLLIMPEXP_CORE wxNotebook;

typedef wxWindow wxDockingPanel;

typedef enum
{
	wxDOCKING_NONE,
	wxDOCKING_WINDOW,
	wxDOCKING_SPLITTER,
	wxDOCKING_NOTEBOOK,
	wxDOCKING_TOOLBAR,
	wxDOCKING_FRAME
} wxDockingPanelType;

#endif // _WX_DOCKING_DEFS_H_
