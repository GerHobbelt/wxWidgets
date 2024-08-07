///////////////////////////////////////////////////////////////////////////////
// Name:        wx/iosfwrap.h
// Purpose:     This file is obsolete, include <iosfwd> directly instead
// Author:      Jan van Dijk <jan@etpmod.phys.tue.nl>
// Created:     18.12.2002
// Copyright:   wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "wx/defs.h"

#if wxUSE_STD_IOSTREAM

#include <iosfwd>

#ifdef __WINDOWS__
#   include "wx/msw/winundef.h"
#endif

#endif // wxUSE_STD_IOSTREAM

