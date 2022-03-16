/////////////////////////////////////////////////////////////////////////////
// Name:        wx/setup-minimize-monolithic-build.h
// Purpose:     depending on one or more defines set in the application build process,
//              tweak the current wxWidgets setup to help produce a minimal binary.
// Author:      Ger Hobbelt
// Modified by:
// Created:     11.01.22
// Copyright:   (c) 2000 Ger Hobbelt
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef   _WX_MINIMIZE_MONOLITHIC_BUILD_H_
#define   _WX_MINIMIZE_MONOLITHIC_BUILD_H_

// set debug level to ZERO for minimal builds without assertion checks, etc.
#define wxDEBUG_LEVEL 12

#if defined(BUILDING_WXWIDGETS_MINIMAL_NO_GUI)

#include "./setup-minimize-monolithic-build-4-console.h"

#elif defined(BUILDING_WXWIDGETS_MINIMAL_WITH_GUI)

#include "./setup-minimize-monolithic-build-4-propgrid.h"

#elif defined(BUILDING_WXWIDGETS_MINIMAL_WITH_AUI_GUI)

#include "./setup-minimize-monolithic-build-4-aui.h"

#endif

#endif 

