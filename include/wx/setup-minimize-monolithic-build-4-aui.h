/////////////////////////////////////////////////////////////////////////////
// Name:        wx/setup-minimize-monolithic-build-4-propgrid.h
// Purpose:     depending on one or more defines set in the application build process,
//              tweak the current wxWidgets setup to help produce a minimal binary.
// Author:      Ger Hobbelt
// Modified by:
// Created:     11.01.22
// Copyright:   (c) 2000 Ger Hobbelt
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef   _WX_MINIMIZE_MONOLITHIC_BUILD_4_PROPGRID_H_
#define   _WX_MINIMIZE_MONOLITHIC_BUILD_4_PROPGRID_H_

#undef wxUSE_GUI
#define wxUSE_GUI            1

#if 0

#undef WXWIN_COMPATIBILITY_2_8
#define WXWIN_COMPATIBILITY_2_8 0

#undef WXWIN_COMPATIBILITY_3_0
#define WXWIN_COMPATIBILITY_3_0 0

#undef wxDIALOG_UNIT_COMPATIBILITY
#define wxDIALOG_UNIT_COMPATIBILITY   0

//#undef wxUSE_UNSAFE_WXSTRING_CONV
//#define wxUSE_UNSAFE_WXSTRING_CONV 0

#undef wxUSE_REPRODUCIBLE_BUILD
#define wxUSE_REPRODUCIBLE_BUILD 0

//#undef wxUSE_ON_FATAL_EXCEPTION
//#define wxUSE_ON_FATAL_EXCEPTION 0

#undef wxUSE_STACKWALKER
#define wxUSE_STACKWALKER 1

#undef wxUSE_DEBUGREPORT
#define wxUSE_DEBUGREPORT 1

#undef wxUSE_DEBUG_CONTEXT
#define wxUSE_DEBUG_CONTEXT 1

#undef wxUSE_MEMORY_TRACING
#define wxUSE_MEMORY_TRACING 0

#undef wxUSE_GLOBAL_MEMORY_OPERATORS
#define wxUSE_GLOBAL_MEMORY_OPERATORS 0

#undef wxUSE_DEBUG_NEW_ALWAYS
#define wxUSE_DEBUG_NEW_ALWAYS 1

//#undef wxUSE_UNICODE
//#define wxUSE_UNICODE 0

//#undef wxUSE_EXCEPTIONS
//#define wxUSE_EXCEPTIONS    0

#undef wxUSE_EXTENDED_RTTI
#define wxUSE_EXTENDED_RTTI 0

//#undef wxUSE_LOG
//#define wxUSE_LOG 0

#undef wxUSE_LOGWINDOW
#define wxUSE_LOGWINDOW 0

#undef wxUSE_LOGGUI
#define wxUSE_LOGGUI 0

#undef wxUSE_LOG_DIALOG
#define wxUSE_LOG_DIALOG 0

//#undef wxUSE_CMDLINE_PARSER
//#define wxUSE_CMDLINE_PARSER 0

#undef wxUSE_THREADS
#define wxUSE_THREADS 0

#undef wxUSE_STREAMS
#define wxUSE_STREAMS       1

//#undef wxUSE_PRINTF_POS_PARAMS
//#define wxUSE_PRINTF_POS_PARAMS      0

//#undef wxUSE_COMPILER_TLS
//#define wxUSE_COMPILER_TLS 0

#undef wxUSE_STL
#define wxUSE_STL 1

#undef wxUSE_STD_DEFAULT
#define wxUSE_STD_DEFAULT  1

//#undef wxUSE_STD_CONTAINERS_COMPATIBLY
//#define wxUSE_STD_CONTAINERS_COMPATIBLY wxUSE_STD_DEFAULT

#undef wxUSE_STD_CONTAINERS
#define wxUSE_STD_CONTAINERS 0

#undef wxUSE_STD_IOSTREAM
#define wxUSE_STD_IOSTREAM  wxUSE_STD_DEFAULT

#undef wxUSE_STD_STRING
#define wxUSE_STD_STRING  wxUSE_STD_DEFAULT

#undef wxUSE_STD_STRING_CONV_IN_WXSTRING
#define wxUSE_STD_STRING_CONV_IN_WXSTRING wxUSE_STL

//#undef wxUSE_LONGLONG
//#define wxUSE_LONGLONG      0

#undef wxUSE_BASE64
#define wxUSE_BASE64        1

#undef wxUSE_CONSOLE_EVENTLOOP
#define wxUSE_CONSOLE_EVENTLOOP 0

#undef wxUSE_FILE
#define wxUSE_FILE          1

#undef wxUSE_FFILE
#define wxUSE_FFILE         1

#undef wxUSE_FSVOLUME
#define wxUSE_FSVOLUME      0

#undef wxUSE_SECRETSTORE
#define wxUSE_SECRETSTORE   0

#undef wxUSE_SPELLCHECK
#define wxUSE_SPELLCHECK 0

#undef wxUSE_STDPATHS
#define wxUSE_STDPATHS      1

#undef wxUSE_TEXTBUFFER
#define wxUSE_TEXTBUFFER    0

#undef wxUSE_TEXTFILE
#define wxUSE_TEXTFILE      0

#undef wxUSE_INTL
#define wxUSE_INTL          0

#undef wxUSE_XLOCALE
#define wxUSE_XLOCALE       0

#undef wxUSE_DATETIME
#define wxUSE_DATETIME      1

#undef wxUSE_TIMER
#define wxUSE_TIMER         1

#undef wxUSE_STOPWATCH
#define wxUSE_STOPWATCH     0

#undef wxUSE_FSWATCHER
#define wxUSE_FSWATCHER     0

#undef wxUSE_CONFIG
#define wxUSE_CONFIG        0

#undef wxUSE_CONFIG_NATIVE
#define wxUSE_CONFIG_NATIVE   0

#undef wxUSE_DIALUP_MANAGER
#define wxUSE_DIALUP_MANAGER   0

//#undef wxUSE_DYNLIB_CLASS
//#define wxUSE_DYNLIB_CLASS    0

//#undef wxUSE_DYNAMIC_LOADER
//#define wxUSE_DYNAMIC_LOADER  0

#undef wxUSE_SOCKETS
#define wxUSE_SOCKETS       0

#undef wxUSE_IPV6
#define wxUSE_IPV6          0

#undef wxUSE_FILESYSTEM
#define wxUSE_FILESYSTEM    1

#undef wxUSE_FS_ZIP
#define wxUSE_FS_ZIP        0

#undef wxUSE_FS_ARCHIVE
#define wxUSE_FS_ARCHIVE    0

#undef wxUSE_FS_INET
#define wxUSE_FS_INET       0

#undef wxUSE_ARCHIVE_STREAMS
#define wxUSE_ARCHIVE_STREAMS     0

#undef wxUSE_ZIPSTREAM
#define wxUSE_ZIPSTREAM     0

#undef wxUSE_TARSTREAM
#define wxUSE_TARSTREAM     0

#undef wxUSE_ZLIB
#define wxUSE_ZLIB          0

#undef wxUSE_LIBLZMA
#define wxUSE_LIBLZMA       0

#undef wxUSE_DOCKING            
#define wxUSE_DOCKING             0

#undef wxUSE_APPLE_IEEE
#define wxUSE_APPLE_IEEE          0

#undef wxUSE_JOYSTICK
#define wxUSE_JOYSTICK            0

#undef wxUSE_FONTENUM
#define wxUSE_FONTENUM 1

#undef wxUSE_FONTMAP
#define wxUSE_FONTMAP 0

#undef wxUSE_MIMETYPE
#define wxUSE_MIMETYPE 0

#undef wxUSE_WEBREQUEST
#define wxUSE_WEBREQUEST 0

#undef wxUSE_WEBREQUEST_URLSESSION
#define wxUSE_WEBREQUEST_URLSESSION 0

#undef wxUSE_WEBREQUEST_CURL
#define wxUSE_WEBREQUEST_CURL 0

#undef wxUSE_PROTOCOL
#define wxUSE_PROTOCOL 0

#undef wxUSE_PROTOCOL_FILE
#define wxUSE_PROTOCOL_FILE 0

#undef wxUSE_PROTOCOL_FTP
#define wxUSE_PROTOCOL_FTP 0

#undef wxUSE_PROTOCOL_HTTP
#define wxUSE_PROTOCOL_HTTP 0

#undef wxUSE_URL
#define wxUSE_URL 0

#undef wxUSE_URL_NATIVE
#define wxUSE_URL_NATIVE 0

#undef wxUSE_VARIANT
#define wxUSE_VARIANT 1

#undef wxUSE_ANY
#define wxUSE_ANY 1

#undef wxUSE_REGEX
#define wxUSE_REGEX       0

#undef wxUSE_SYSTEM_OPTIONS
#define wxUSE_SYSTEM_OPTIONS 0

#undef wxUSE_SOUND
#define wxUSE_SOUND      0

#undef wxUSE_MEDIACTRL     
#define wxUSE_MEDIACTRL     0

#undef wxUSE_XRC       
#define wxUSE_XRC       0

#undef wxUSE_XML
#define wxUSE_XML       0

#undef wxUSE_AUI       
#define wxUSE_AUI       1

#undef wxUSE_RIBBON    
#define wxUSE_RIBBON    0

#undef wxUSE_PROPGRID  
#define wxUSE_PROPGRID  1

#undef wxUSE_STC 
#define wxUSE_STC 0

#undef wxUSE_WEBVIEW 
#define wxUSE_WEBVIEW 0

#undef wxUSE_WEBVIEW_IE 
#define wxUSE_WEBVIEW_IE 0

#undef wxUSE_WEBVIEW_EDGE 
#define wxUSE_WEBVIEW_EDGE 0

#undef wxUSE_WEBVIEW_WEBKIT 
#define wxUSE_WEBVIEW_WEBKIT 0

#undef wxUSE_WEBVIEW_WEBKIT2 
#define wxUSE_WEBVIEW_WEBKIT2 0

#undef wxUSE_GRAPHICS_CONTEXT 
#define wxUSE_GRAPHICS_CONTEXT 0

#undef wxUSE_CAIRO 
#define wxUSE_CAIRO 0

#undef wxUSE_CONTROLS     
#define wxUSE_CONTROLS     1

#undef wxUSE_MARKUP       
#define wxUSE_MARKUP       0

#undef wxUSE_POPUPWIN     
#define wxUSE_POPUPWIN     1

#undef wxUSE_TIPWINDOW    
#define wxUSE_TIPWINDOW    0

#undef wxUSE_ACTIVITYINDICATOR
#define wxUSE_ACTIVITYINDICATOR 0

#undef wxUSE_ANIMATIONCTRL
#define wxUSE_ANIMATIONCTRL 0

#undef wxUSE_BANNERWINDOW
#define wxUSE_BANNERWINDOW  0

#undef wxUSE_BUTTON
#define wxUSE_BUTTON        1

#undef wxUSE_BMPBUTTON
#define wxUSE_BMPBUTTON     1

#undef wxUSE_CALENDARCTRL
#define wxUSE_CALENDARCTRL  0

#undef wxUSE_CHECKBOX
#define wxUSE_CHECKBOX      1

#undef wxUSE_CHECKLISTBOX
#define wxUSE_CHECKLISTBOX  0

#undef wxUSE_CHOICE
#define wxUSE_CHOICE        1

#undef wxUSE_COLLPANE
#define wxUSE_COLLPANE      0

#undef wxUSE_COLOURPICKERCTRL
#define wxUSE_COLOURPICKERCTRL 0

#undef wxUSE_COMBOBOX
#define wxUSE_COMBOBOX      1

#undef wxUSE_COMMANDLINKBUTTON
#define wxUSE_COMMANDLINKBUTTON 0

#undef wxUSE_DATAVIEWCTRL
#define wxUSE_DATAVIEWCTRL  0

#undef wxUSE_DATEPICKCTRL
#define wxUSE_DATEPICKCTRL  0

#undef wxUSE_DIRPICKERCTRL
#define wxUSE_DIRPICKERCTRL 0

#undef wxUSE_EDITABLELISTBOX
#define wxUSE_EDITABLELISTBOX 1

#undef wxUSE_FILECTRL
#define wxUSE_FILECTRL      1

#undef wxUSE_FILEPICKERCTRL
#define wxUSE_FILEPICKERCTRL 0

#undef wxUSE_FONTPICKERCTRL
#define wxUSE_FONTPICKERCTRL 0

#undef wxUSE_GAUGE
#define wxUSE_GAUGE         0

#undef wxUSE_HEADERCTRL
#define wxUSE_HEADERCTRL    1

#undef wxUSE_HYPERLINKCTRL
#define wxUSE_HYPERLINKCTRL 0

#undef wxUSE_LISTBOX
#define wxUSE_LISTBOX       1

#undef wxUSE_LISTCTRL
#define wxUSE_LISTCTRL      1

#undef wxUSE_RADIOBOX
#define wxUSE_RADIOBOX      0

#undef wxUSE_RADIOBTN
#define wxUSE_RADIOBTN      0

#undef wxUSE_RICHMSGDLG
#define wxUSE_RICHMSGDLG    0

#undef wxUSE_SCROLLBAR
#define wxUSE_SCROLLBAR     0

#undef wxUSE_SEARCHCTRL
#define wxUSE_SEARCHCTRL    0

#undef wxUSE_SLIDER
#define wxUSE_SLIDER        0

#undef wxUSE_SPINBTN
#define wxUSE_SPINBTN       1

#undef wxUSE_SPINCTRL
#define wxUSE_SPINCTRL      1

#undef wxUSE_STATBOX
#define wxUSE_STATBOX       0

#undef wxUSE_STATLINE
#define wxUSE_STATLINE      0

#undef wxUSE_STATTEXT
#define wxUSE_STATTEXT      1

#undef wxUSE_STATBMP
#define wxUSE_STATBMP       0

#undef wxUSE_TEXTCTRL
#define wxUSE_TEXTCTRL      1

#undef wxUSE_TIMEPICKCTRL
#define wxUSE_TIMEPICKCTRL  0

#undef wxUSE_TOGGLEBTN
#define wxUSE_TOGGLEBTN     0

#undef wxUSE_TREECTRL
#define wxUSE_TREECTRL      1

#undef wxUSE_TREELISTCTRL
#define wxUSE_TREELISTCTRL  0

#undef wxUSE_NATIVE_DATAVIEWCTRL
#define wxUSE_NATIVE_DATAVIEWCTRL 0

#undef wxUSE_STATUSBAR
#define wxUSE_STATUSBAR    1

#undef wxUSE_NATIVE_STATUSBAR
#define wxUSE_NATIVE_STATUSBAR        1

#undef wxUSE_TOOLBAR
#define wxUSE_TOOLBAR 1

#undef wxUSE_TOOLBAR_NATIVE
#define wxUSE_TOOLBAR_NATIVE 1

#undef wxUSE_NOTEBOOK
#define wxUSE_NOTEBOOK 0

#undef wxUSE_LISTBOOK
#define wxUSE_LISTBOOK 0

#undef wxUSE_CHOICEBOOK
#define wxUSE_CHOICEBOOK 0

#undef wxUSE_TREEBOOK
#define wxUSE_TREEBOOK 0

#undef wxUSE_TOOLBOOK
#define wxUSE_TOOLBOOK 0

#undef wxUSE_TASKBARICON
#define wxUSE_TASKBARICON 0

#undef wxUSE_GRID
#define wxUSE_GRID         1

#undef wxUSE_MINIFRAME
#define wxUSE_MINIFRAME 0

#undef wxUSE_COMBOCTRL
#define wxUSE_COMBOCTRL 1

#undef wxUSE_ODCOMBOBOX
#define wxUSE_ODCOMBOBOX 1

#undef wxUSE_BITMAPCOMBOBOX
#define wxUSE_BITMAPCOMBOBOX 0

#undef wxUSE_REARRANGECTRL
#define wxUSE_REARRANGECTRL 0

#undef wxUSE_ADDREMOVECTRL
#define wxUSE_ADDREMOVECTRL 0

#undef wxUSE_MASKED_EDIT
#define wxUSE_MASKED_EDIT 0

#undef wxUSE_ACCEL
#define wxUSE_ACCEL 1

#undef wxUSE_ARTPROVIDER_STD
#define wxUSE_ARTPROVIDER_STD 0

#undef wxUSE_ARTPROVIDER_TANGO
#define wxUSE_ARTPROVIDER_TANGO 0

#undef wxUSE_HOTKEY
#define wxUSE_HOTKEY 1

#undef wxUSE_CARET
#define wxUSE_CARET         0

#undef wxUSE_DISPLAY
#define wxUSE_DISPLAY       0

#undef wxUSE_GEOMETRY
#define wxUSE_GEOMETRY            0

#undef wxUSE_IMAGLIST
#define wxUSE_IMAGLIST      1

#undef wxUSE_INFOBAR
#define wxUSE_INFOBAR       0

#undef wxUSE_MENUS
#define wxUSE_MENUS         1

#undef wxUSE_MENUBAR
#define wxUSE_MENUBAR       1

#undef wxUSE_NOTIFICATION_MESSAGE
#define wxUSE_NOTIFICATION_MESSAGE 0

#undef wxUSE_PREFERENCES_EDITOR
#define wxUSE_PREFERENCES_EDITOR 0

#undef wxUSE_PRIVATE_FONTS
#define wxUSE_PRIVATE_FONTS 0

#undef wxUSE_RICHTOOLTIP
#define wxUSE_RICHTOOLTIP 0

#undef wxUSE_SASH
#define wxUSE_SASH          1

#undef wxUSE_SPLITTER
#define wxUSE_SPLITTER      1

#undef wxUSE_TOOLTIPS
#define wxUSE_TOOLTIPS      0

#undef wxUSE_VALIDATORS
#define wxUSE_VALIDATORS 0

#undef wxUSE_AUTOID_MANAGEMENT
#define wxUSE_AUTOID_MANAGEMENT 0

#undef wxUSE_COMMON_DIALOGS
#define wxUSE_COMMON_DIALOGS 0

#undef wxUSE_BUSYINFO
#define wxUSE_BUSYINFO      0

#undef wxUSE_CHOICEDLG
#define wxUSE_CHOICEDLG     1

#undef wxUSE_COLOURDLG
#define wxUSE_COLOURDLG     1

#undef wxUSE_DIRDLG
#define wxUSE_DIRDLG 1

#undef wxUSE_FILEDLG
#define wxUSE_FILEDLG       1

#undef wxUSE_FINDREPLDLG
#define wxUSE_FINDREPLDLG       0

#undef wxUSE_FONTDLG
#define wxUSE_FONTDLG       0

#undef wxUSE_MSGDLG
#define wxUSE_MSGDLG        1

#undef wxUSE_PROGRESSDLG
#define wxUSE_PROGRESSDLG 0

#undef wxUSE_NATIVE_PROGRESSDLG
#define wxUSE_NATIVE_PROGRESSDLG 0

#undef wxUSE_STARTUP_TIPS
#define wxUSE_STARTUP_TIPS 0

#undef wxUSE_TEXTDLG
#define wxUSE_TEXTDLG 1

#undef wxUSE_NUMBERDLG
#define wxUSE_NUMBERDLG 1

#undef wxUSE_CREDENTIALDLG
#define wxUSE_CREDENTIALDLG 0

#undef wxUSE_SPLASH
#define wxUSE_SPLASH 0

#undef wxUSE_WIZARDDLG
#define wxUSE_WIZARDDLG 0

#undef wxUSE_ABOUTDLG
#define wxUSE_ABOUTDLG 1

#undef wxUSE_FILE_HISTORY
#define wxUSE_FILE_HISTORY 0

#undef wxUSE_METAFILE
#define wxUSE_METAFILE              0

#undef wxUSE_ENH_METAFILE
#define wxUSE_ENH_METAFILE          0

#undef wxUSE_WIN_METAFILES_ALWAYS
#define wxUSE_WIN_METAFILES_ALWAYS  0

#undef wxUSE_MDI
#define wxUSE_MDI 0

#undef wxUSE_DOC_VIEW_ARCHITECTURE
#define wxUSE_DOC_VIEW_ARCHITECTURE 0

#undef wxUSE_MDI_ARCHITECTURE
#define wxUSE_MDI_ARCHITECTURE    0

#undef wxUSE_PRINTING_ARCHITECTURE
#define wxUSE_PRINTING_ARCHITECTURE  0

#undef wxUSE_HTML
#define wxUSE_HTML          1

#undef wxUSE_GLCANVAS
#define wxUSE_GLCANVAS       0

#undef wxUSE_GLCANVAS_EGL
#define wxUSE_GLCANVAS_EGL   0

#undef wxUSE_RICHTEXT
#define wxUSE_RICHTEXT       0

#undef wxUSE_CLIPBOARD
#define wxUSE_CLIPBOARD     0

#undef wxUSE_DATAOBJ
#define wxUSE_DATAOBJ       0

#undef wxUSE_DRAG_AND_DROP
#define wxUSE_DRAG_AND_DROP 0

#undef wxUSE_ACCESSIBILITY
#define wxUSE_ACCESSIBILITY 0

#undef wxUSE_SNGLINST_CHECKER
#define wxUSE_SNGLINST_CHECKER  0

#undef wxUSE_DRAGIMAGE
#define wxUSE_DRAGIMAGE 0

#undef wxUSE_IPC
#define wxUSE_IPC         0

#undef wxUSE_HELP
#define wxUSE_HELP        0

#undef wxUSE_MS_HTML_HELP
#define wxUSE_MS_HTML_HELP 0

#undef wxUSE_WXHTML_HELP
#define wxUSE_WXHTML_HELP 0

#undef wxUSE_CONSTRAINTS
#define wxUSE_CONSTRAINTS 0

#undef wxUSE_SPLINES
#define wxUSE_SPLINES     0

#undef wxUSE_MOUSEWHEEL
#define wxUSE_MOUSEWHEEL        0

#undef wxUSE_UIACTIONSIMULATOR
#define wxUSE_UIACTIONSIMULATOR 0

#undef wxUSE_POSTSCRIPT
#define wxUSE_POSTSCRIPT  0

#undef wxUSE_AFM_FOR_POSTSCRIPT
#define wxUSE_AFM_FOR_POSTSCRIPT 0

#undef wxUSE_SVG
#define wxUSE_SVG 1

#undef wxUSE_DC_TRANSFORM_MATRIX
#define wxUSE_DC_TRANSFORM_MATRIX 0

#undef wxUSE_IMAGE
#define wxUSE_IMAGE         1

#undef wxUSE_LIBPNG
#define wxUSE_LIBPNG        1

#undef wxUSE_LIBJPEG
#define wxUSE_LIBJPEG       1

#undef wxUSE_LIBTIFF
#define wxUSE_LIBTIFF       0

#undef wxUSE_TGA
#define wxUSE_TGA           0

#undef wxUSE_GIF
#define wxUSE_GIF           1

#undef wxUSE_PNM
#define wxUSE_PNM           0

#undef wxUSE_PCX
#define wxUSE_PCX           0

#undef wxUSE_IFF
#define wxUSE_IFF           0

#undef wxUSE_XPM
#define wxUSE_XPM           1

#undef wxUSE_ICO_CUR
#define wxUSE_ICO_CUR       1

#undef wxUSE_PALETTE
#define wxUSE_PALETTE       1

#undef wxUSE_ALL_THEMES
#define wxUSE_ALL_THEMES    0

#undef wxUSE_THEME_GTK
#define wxUSE_THEME_GTK     0

#undef wxUSE_THEME_METAL
#define wxUSE_THEME_METAL   0

#undef wxUSE_THEME_MONO
#define wxUSE_THEME_MONO    0

#undef wxUSE_THEME_WIN32
#define wxUSE_THEME_WIN32   0

#undef wxUSE_GSTREAMER
#define wxUSE_GSTREAMER 0

#undef wxUSE_GSTREAMER_PLAYER
#define wxUSE_GSTREAMER_PLAYER 0

#undef wxUSE_XTEST
#define wxUSE_XTEST 0

//#undef wxUSE_GRAPHICS_GDIPLUS
//#define wxUSE_GRAPHICS_GDIPLUS wxUSE_GRAPHICS_CONTEXT

//#undef wxUSE_GRAPHICS_DIRECT2D
//#define wxUSE_GRAPHICS_DIRECT2D wxUSE_GRAPHICS_CONTEXT

#undef wxUSE_WEBREQUEST_WINHTTP
#define wxUSE_WEBREQUEST_WINHTTP 0

#undef wxUSE_OLE
#define wxUSE_OLE           0

#undef wxUSE_OLE_AUTOMATION
#define wxUSE_OLE_AUTOMATION 0

#undef wxUSE_ACTIVEX
#define wxUSE_ACTIVEX 0

#undef wxUSE_WINRT
#define wxUSE_WINRT 0

#undef wxUSE_DC_CACHEING
#define wxUSE_DC_CACHEING 01

#undef wxUSE_WXDIB
#define wxUSE_WXDIB 0

#undef wxUSE_REGKEY
#define wxUSE_REGKEY 0

#undef wxUSE_RICHEDIT
#define wxUSE_RICHEDIT  0

#undef wxUSE_RICHEDIT2
#define wxUSE_RICHEDIT2 0

#undef wxUSE_OWNER_DRAWN
#define wxUSE_OWNER_DRAWN 1

#undef wxUSE_TASKBARICON_BALLOONS
#define wxUSE_TASKBARICON_BALLOONS 0

#undef wxUSE_TASKBARBUTTON
#define wxUSE_TASKBARBUTTON 0

#undef wxUSE_UXTHEME
#define wxUSE_UXTHEME           0

#undef wxUSE_INKEDIT
#define wxUSE_INKEDIT  0

#undef wxUSE_INICONF
#define wxUSE_INICONF 0

#undef wxUSE_WINSOCK2
#define wxUSE_WINSOCK2 0

#undef wxUSE_DATEPICKCTRL_GENERIC
#define wxUSE_DATEPICKCTRL_GENERIC 0

#undef wxUSE_TIMEPICKCTRL_GENERIC
#define wxUSE_TIMEPICKCTRL_GENERIC 0

#undef wxUSE_DBGHELP
#define wxUSE_DBGHELP 0

#undef wxUSE_CRASHREPORT
#define wxUSE_CRASHREPORT 0

#undef wxUSE_LIBSDL
#define wxUSE_LIBSDL 0

#undef wxUSE_PLUGINS
#define wxUSE_PLUGINS 0

#undef wxUSE_GTKPRINT
#define wxUSE_GTKPRINT 0

#undef wxUSE_LIBGNOMEVFS
#define wxUSE_LIBGNOMEVFS 0

#undef wxUSE_LIBNOTIFY
#define wxUSE_LIBNOTIFY 0

#undef wxUSE_OPENGL
#define wxUSE_OPENGL 0

#undef wxUSE_WEBKIT
#define wxUSE_WEBKIT 0

#endif

#endif 

