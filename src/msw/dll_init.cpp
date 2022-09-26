/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/dll_init.cpp
// Purpose:     initialisation for the library
// Author:      Vadim Zeitlin
// Modified by:
// Created:     04.10.99
// Copyright:   (c) Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "wx/wxprec.h"


#ifndef WX_PRECOMP
#include "wx/app.h"
#include "wx/filefn.h"
#include "wx/log.h"
#include "wx/intl.h"
#include "wx/module.h"
#endif

#include "wx/init.h"
#include "wx/thread.h"

#include "wx/scopedptr.h"
#include "wx/except.h"

#if defined(__WINDOWS__) && defined(__VISUALC__)
#include "wx/msw/private.h"
#include "wx/msw/msvcrt.h"

#ifdef wxCrtSetDbgFlag

#pragma init_seg(compiler)

static const unsigned char* wx_crtdbg_purpose_stack[100] = { 0 };
static const unsigned char* wx_crtdbg_purpose_lu_table[100000] = { 0 };

static _CRT_ALLOC_HOOK old_alloc_hook = NULL;

// _CRT_ALLOC_HOOK
static int __CRTDECL alloc_hook_f(int allocType, void* userData, size_t size, int blockType, long requestNumber, const unsigned char* filename, int lineNumber)
{
	switch (allocType)
	{
	case _HOOK_ALLOC:
		break;
	case _HOOK_REALLOC:
		break;
	case _HOOK_FREE:
		break;
	}
	int rv = (old_alloc_hook != NULL ? old_alloc_hook(allocType, userData, size, blockType, requestNumber, filename, lineNumber) : TRUE);
	return rv;
}

static int EnableMemLeakChecking(void)
{
	// check for memory leaks on program exit (another useful flag
	// is _CRTDBG_DELAY_FREE_MEM_DF which doesn't free deallocated
	// memory which may be used to simulate low-memory condition)

	//_CrtSetBreakAlloc(744);  /* Break at memalloc{744}, or 'watch' _crtBreakAlloc */
	//const int desired_flags = (_CRTDBG_ALLOC_MEM_DF | _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	const int desired_flags = (_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	wxCrtSetDbgFlag(desired_flags);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);

	old_alloc_hook = _CrtSetAllocHook(alloc_hook_f);

	return 1;
}

static int gs_memleak_checking_is_set_up = EnableMemLeakChecking();

#endif // wxCrtSetDbgFlag
#endif // __WINDOWS__ + __VISUALC__

