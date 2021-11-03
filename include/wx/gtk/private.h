///////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk/private.h
// Purpose:     wxGTK private macros, functions &c
// Author:      Vadim Zeitlin
// Modified by:
// Created:     12.03.02
// Copyright:   (c) 2002 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GTK_PRIVATE_H_
#define _WX_GTK_PRIVATE_H_

#include "wx/gtk/private/wrapgtk.h"

#include "wx/gtk/private/string.h"

#ifndef G_VALUE_INIT
    // introduced in GLib 2.30
    #define G_VALUE_INIT { 0, { { 0 } } }
#endif

// pango_version_check symbol is quite recent ATM (4/2007)... so we
// use our own wrapper which implements a smart trick.
// Use this function as you'd use pango_version_check:
//
//  if (!wx_pango_version_check(1,18,0))
//     ... call to a function available only in pango >= 1.18 ...
//
// and use it only to test for pango versions >= 1.16.0
extern const gchar *wx_pango_version_check(int major, int minor, int micro);

#define wxGTK_CONV(s) (s).utf8_str()
#define wxGTK_CONV_ENC(s, enc) wxGTK_CONV((s))
#define wxGTK_CONV_FONT(s, font) wxGTK_CONV((s))
#define wxGTK_CONV_SYS(s) wxGTK_CONV((s))

#define wxGTK_CONV_BACK(s) wxString::FromUTF8Unchecked(s)
#define wxGTK_CONV_BACK_ENC(s, enc) wxGTK_CONV_BACK(s)
#define wxGTK_CONV_BACK_FONT(s, font) wxGTK_CONV_BACK(s)
#define wxGTK_CONV_BACK_SYS(s) wxGTK_CONV_BACK(s)

// Define a macro for converting wxString to char* in appropriate encoding for
// the file names.
#ifdef G_OS_WIN32
    // Under MSW, UTF-8 file name encodings are always used.
    #define wxGTK_CONV_FN(s) (s).utf8_str()
#else
    // Under Unix use GLib file name encoding (which is also UTF-8 by default
    // but may be different from it).
    #define wxGTK_CONV_FN(s) (s).fn_str()
#endif

// ----------------------------------------------------------------------------
// various private helper functions
// ----------------------------------------------------------------------------

namespace wxGTKPrivate
{

// these functions create the GTK widgets of the specified types which can then
// used to retrieve their styles, pass them to drawing functions &c
//
// the returned widgets shouldn't be destroyed, this is done automatically on
// shutdown
GtkWidget *GetButtonWidget();
GtkWidget *GetCheckButtonWidget();
GtkWidget *GetComboBoxWidget();
GtkWidget *GetEntryWidget();
GtkWidget *GetHeaderButtonWidgetFirst();
GtkWidget *GetHeaderButtonWidgetLast();
GtkWidget *GetHeaderButtonWidget();
GtkWidget *GetNotebookWidget();
GtkWidget *GetRadioButtonWidget();
GtkWidget *GetSplitterWidget(wxOrientation orient = wxHORIZONTAL);
GtkWidget *GetTextEntryWidget();
GtkWidget *GetTreeWidget();

} // wxGTKPrivate

#endif // _WX_GTK_PRIVATE_H_
