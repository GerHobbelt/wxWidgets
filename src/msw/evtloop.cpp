///////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/evtloop.cpp
// Purpose:     implements wxEventLoop for wxMSW port
// Author:      Vadim Zeitlin
// Modified by:
// Created:     01.06.01
// Copyright:   (c) 2001 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/evtloop.h"

#ifndef WX_PRECOMP
    #include "wx/window.h"
    #include "wx/app.h"
    #include "wx/log.h"
#endif //WX_PRECOMP

#include "wx/thread.h"
#include "wx/except.h"
#include "wx/msw/private.h"

#include "wx/tooltip.h"
#if wxUSE_THREADS
    // define the list of MSG strutures
    WX_DECLARE_LIST(MSG, wxMsgList);

    #include "wx/listimpl.cpp"

    WX_DEFINE_LIST(wxMsgList)
#endif // wxUSE_THREADS

// ============================================================================
// GUI wxEventLoop implementation
// ============================================================================

wxWindowMSW *wxGUIEventLoop::ms_winCritical = NULL;

bool wxGUIEventLoop::IsChildOfCriticalWindow(wxWindowMSW *win)
{
    while ( win )
    {
        if ( win == ms_winCritical )
            return true;

        win = win->GetParent();
    }

    return false;
}

bool wxGUIEventLoop::PreProcessMessage(WXMSG *msg)
{
    HWND hwnd = msg->hwnd;
    wxWindow *wndThis = wxGetWindowFromHWND((WXHWND)hwnd);
    wxWindow *wnd;

    // this might happen if we're in a modeless dialog, or if a wx control has
    // children which themselves were not created by wx (i.e. wxActiveX control children)
    if ( !wndThis )
    {
        // @UE3 2007-16-11: Unreal Engine integration.
        // We don't want pre- processing if this is an Unreal window handle without a wxWindow association.
        // Make certain we have a callback class
        wxCHECK(NULL != GetUnrealCallbacks(), false);
        if (GetUnrealCallbacks()->IsUnrealWindowHandle(hwnd) == true)
        {
            return false;
        }

        while ( hwnd && (::GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD ))
        {
            hwnd = ::GetParent(hwnd);

            // If the control has a wx parent, break and give the parent a chance
            // to process the window message
            wndThis = wxGetWindowFromHWND((WXHWND)hwnd);
            if (wndThis != NULL)
                break;
        }

        if ( !wndThis )
        {
            // this may happen if the event occurred in a standard modeless dialog (the
            // only example of which I know of is the find/replace dialog) - then call
            // IsDialogMessage() to make TAB navigation in it work

            // NOTE: IsDialogMessage() just eats all the messages (i.e. returns true for
            // them) if we call it for the control itself
            return hwnd && ::IsDialogMessage(hwnd, msg) != 0;
        }
    }

    if ( !AllowProcessing(wndThis) )
    {
        // not a child of critical window, so we eat the event but take care to
        // stop an endless stream of WM_PAINTs which would have resulted if we
        // didn't validate the invalidated part of the window
        if ( msg->message == WM_PAINT )
            ::ValidateRect(hwnd, NULL);

        return true;
    }

#if wxUSE_TOOLTIPS
    // we must relay WM_MOUSEMOVE events to the tooltip ctrl if we want it to
    // popup the tooltip bubbles
    if ( msg->message == WM_MOUSEMOVE )
    {
        // we should do it if one of window children has an associated tooltip
        // (and not just if the window has a tooltip itself)
        if ( wndThis->HasToolTips() )
            wxToolTip::RelayEvent((WXMSG *)msg);
    }
#endif // wxUSE_TOOLTIPS

    // allow the window to prevent certain messages from being
    // translated/processed (this is currently used by wxTextCtrl to always
    // grab Ctrl-C/V/X, even if they are also accelerators in some parent)
    if ( !wndThis->MSWShouldPreProcessMessage((WXMSG *)msg) )
    {
        return false;
    }

    // try translations first: the accelerators override everything
    for ( wnd = wndThis; wnd; wnd = wnd->GetParent() )
    {
        if ( wnd->MSWTranslateMessage((WXMSG *)msg))
            return true;

        // stop at top navigation domain, i.e. typically a top level window
        if ( wnd->IsTopNavigationDomain(wxWindow::Navigation_Accel) )
            break;
    }

    // now try the other hooks (kbd navigation is handled here)
    for ( wnd = wndThis; wnd; wnd = wnd->GetParent() )
    {
        if ( wnd->MSWProcessMessage((WXMSG *)msg) )
            return true;

        // also stop at first top level window here, just as above because
        // if we don't do this, pressing ESC on a modal dialog shown as child
        // of a modal dialog with wxID_CANCEL will cause the parent dialog to
        // be closed, for example
        if ( wnd->IsTopNavigationDomain(wxWindow::Navigation_Accel) )
            break;
    }

    // no special preprocessing for this message, dispatch it normally
    return false;
}

void wxGUIEventLoop::ProcessMessage(WXMSG *msg)
{
    // give us the chance to preprocess the message first
    if ( !PreProcessMessage(msg) )
    {
        // if it wasn't done, dispatch it to the corresponding window
        ::TranslateMessage(msg);
        ::DispatchMessage(msg);
    }
}

bool wxGUIEventLoop::Dispatch()
{
    MSG msg;
    if ( !GetNextMessage(&msg) )
        return false;

#if wxUSE_THREADS
    wxASSERT_MSG( wxThread::IsMain(),
                  wxT("only the main thread can process Windows messages") );

    static bool s_hadGuiLock = true;
    static wxMsgList s_aSavedMessages;

    // if a secondary thread owning the mutex is doing GUI calls, save all
    // messages for later processing - we can't process them right now because
    // it will lead to recursive library calls (and we're not reentrant)
    if ( !wxGuiOwnedByMainThread() )
    {
        s_hadGuiLock = false;

        // leave out WM_COMMAND messages: too dangerous, sometimes
        // the message will be processed twice
        if ( !wxIsWaitingForThread() || msg.message != WM_COMMAND )
        {
            MSG* pMsg = new MSG(msg);
            s_aSavedMessages.Append(pMsg);
        }

        return true;
    }
    else
    {
        // have we just regained the GUI lock? if so, post all of the saved
        // messages
        //
        // FIXME of course, it's not _exactly_ the same as processing the
        //       messages normally - expect some things to break...
        if ( !s_hadGuiLock )
        {
            s_hadGuiLock = true;

            wxMsgList::compatibility_iterator node = s_aSavedMessages.GetFirst();
            while (node)
            {
                MSG* pMsg = node->GetData();
                s_aSavedMessages.Erase(node);

                ProcessMessage(pMsg);
                delete pMsg;

                node = s_aSavedMessages.GetFirst();
            }
        }
    }
#endif // wxUSE_THREADS

    ProcessMessage(&msg);

    return true;
}

int wxGUIEventLoop::DispatchTimeout(unsigned long timeout)
{
    MSG msg;
    int rc = GetNextMessageTimeout(&msg, timeout);
    if ( rc != 1 )
        return rc;

    ProcessMessage(&msg);

    return 1;
}

void wxGUIEventLoop::OnNextIteration()
{
#if wxUSE_THREADS
    wxMutexGuiLeaveOrEnter();
#endif // wxUSE_THREADS
}


// ----------------------------------------------------------------------------
// Yield to incoming messages
// ----------------------------------------------------------------------------

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(wxMSGArray);

void wxGUIEventLoop::DoYieldFor(long eventsToProcess)
{
    // we don't want to process WM_QUIT from here - it should be processed in
    // the main event loop in order to stop it
    MSG msg;
    int nPaintsReceived = 0;
    while ( PeekMessage(&msg, (HWND)0, 0, 0, PM_NOREMOVE) &&
            msg.message != WM_QUIT )
    {
#if wxUSE_THREADS
        wxMutexGuiLeaveOrEnter();
#endif // wxUSE_THREADS

        if (msg.message == WM_PAINT)
        {
            // NOTE: WM_PAINTs are categorized as wxEVT_CATEGORY_UI
            if ((eventsToProcess & wxEVT_CATEGORY_UI) == 0)
            {
                // this msg is not going to be dispatched...
                // however WM_PAINT is special: until there are damaged
                // windows, Windows will keep sending it forever!
                if (nPaintsReceived > 10)
                {
                    // we got 10 WM_PAINT consecutive messages...
                    // we must have reached the tail of the message queue:
                    // we're now getting _only_ WM_PAINT events and this will
                    // continue forever (since we don't dispatch them
                    // because of the user-specified eventsToProcess mask)...
                    // break out of this loop!
                    break;
                }
                else
                    nPaintsReceived++;
            }
            //else: we're going to dispatch it below,
            //      so we don't need to take any special action
        }
        else
        {
            // reset the counter of consecutive WM_PAINT messages received:
            nPaintsReceived = 0;
        }

        // choose a wxEventCategory for this Windows message
        bool processNow;
        switch (msg.message)
        {
            case WM_NCMOUSEMOVE:

            case WM_NCLBUTTONDOWN:
            case WM_NCLBUTTONUP:
            case WM_NCLBUTTONDBLCLK:
            case WM_NCRBUTTONDOWN:
            case WM_NCRBUTTONUP:
            case WM_NCRBUTTONDBLCLK:
            case WM_NCMBUTTONDOWN:
            case WM_NCMBUTTONUP:
            case WM_NCMBUTTONDBLCLK:

            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_CHAR:
            case WM_DEADCHAR:
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_SYSCHAR:
            case WM_SYSDEADCHAR:
            case WM_UNICHAR:
            case WM_HOTKEY:
            case WM_IME_STARTCOMPOSITION:
            case WM_IME_ENDCOMPOSITION:
            case WM_IME_COMPOSITION:
            case WM_COMMAND:
            case WM_SYSCOMMAND:

            case WM_IME_SETCONTEXT:
            case WM_IME_NOTIFY:
            case WM_IME_CONTROL:
            case WM_IME_COMPOSITIONFULL:
            case WM_IME_SELECT:
            case WM_IME_CHAR:
            case WM_IME_KEYDOWN:
            case WM_IME_KEYUP:

            case WM_MOUSEHOVER:
            case WM_MOUSELEAVE:
            case WM_NCMOUSELEAVE:

            case WM_CUT:
            case WM_COPY:
            case WM_PASTE:
            case WM_CLEAR:
            case WM_UNDO:

            case WM_MOUSEMOVE:
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_LBUTTONDBLCLK:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_RBUTTONDBLCLK:
            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
            case WM_MBUTTONDBLCLK:
            case WM_MOUSEWHEEL:
                processNow = (eventsToProcess & wxEVT_CATEGORY_USER_INPUT) != 0;
                break;

            case WM_TIMER:
                processNow = (eventsToProcess & wxEVT_CATEGORY_TIMER) != 0;
                break;

            default:
                if (msg.message < WM_USER)
                {
                    // 0;WM_USER-1 is the range of message IDs reserved for use
                    // by the system.
                    // there are too many of these types of messages to handle
                    // them in this switch
                    processNow = (eventsToProcess & wxEVT_CATEGORY_UI) != 0;
                }
                else
                {
                    // Process all the unknown messages. We must do it because
                    // failure to process some of them can be fatal, e.g. if we
                    // don't dispatch WM_APP+2 then embedded IE ActiveX
                    // controls don't work any more, see #14027. And there may
                    // be more examples like this, so dispatch all unknown
                    // messages immediately to be safe.
                    processNow = true;
                }
        }

        // should we process this event now?
        if ( processNow )
        {
            if ( !wxTheApp->Dispatch() )
                break;
        }
        else
        {
            // remove the message and store it
            ::GetMessage(&msg, NULL, 0, 0);
            m_arrMSG.Add(msg);
        }
    }

    wxEventLoopBase::DoYieldFor(eventsToProcess);

    // put back unprocessed events in the queue
    DWORD id = GetCurrentThreadId();
    for (size_t i=0; i<m_arrMSG.GetCount(); i++)
    {
        PostThreadMessage(id, m_arrMSG[i].message,
                          m_arrMSG[i].wParam, m_arrMSG[i].lParam);
    }

    m_arrMSG.Clear();
}

// @UE3 2007-16-11: Unreal Engine integration.

// @UE3 14-01-2024: Added a stub here in case we don't have a callback
class wxUnrealCallbacksStubbed : public wxUnrealCallbacks
{
public:
    bool IsUnrealWindowHandle(HWND) const override { return false; }
    bool IsRequestingExit() const override { return m_requestingExit; }
    void SetRequestingExit(bool requestingExit) { m_requestingExit = requestingExit; }

#if WITH_UE3_CRASH_HANDLING
    int WndProcExceptionFilter(LPEXCEPTION_POINTERS) override { return EXCEPTION_EXECUTE_HANDLER; }
    void WndProcUnhandledExceptionCallback() override { }
#endif

private:
    bool m_requestingExit = false;
};
static wxUnrealCallbacksStubbed s_UnrealCallbacks;

int wxGUIEventLoop::MainRun()
{
    // event loops are not recursive, you need to create another loop!
    wxCHECK_MSG(!IsRunning(), -1, _T("can't reenter a message loop"));

    // ProcessIdle() and Dispatch() below may throw so the code here should
    // be exception-safe, hence we must use local objects for all actions we
    // should undo
//	wxEventLoopActivator activate(ms_activeLoop);
    wxEventLoopActivator activate(wx_static_cast(wxEventLoop*, this));

    // Set this variable to true for the duration of this method.
    m_isInsideRun = true;
    wxON_BLOCK_EXIT_SET(m_isInsideRun, false);

    //wxRunningEventLoopCounter evtLoopCounter;

    // If we don't have YE callbacks, fill in with a subsitute instead
    if (GetUnrealCallbacks() == nullptr)
    {
        SetUnrealCallbacks(&s_UnrealCallbacks);
    }

    // we must ensure that OnExit() is called even if an exception is thrown
    // from inside Dispatch() but we must call it from Exit() in normal
    // situations because it is supposed to be called synchronously,
    // wxModalEventLoop depends on this (so we can't just use ON_BLOCK_EXIT or
    // something similar here)
#if wxUSE_EXCEPTIONS
    for (;; )
    {
        try
        {
#error DONT_USE_EXCEPTIONS_WITH_UNREAL_ENGINE
#endif // wxUSE_EXCEPTIONS

            // this is the event loop itself
            for (;; )
            {
#if wxUSE_THREADS
                wxMutexGuiLeaveOrEnter();
#endif // wxUSE_THREADS

                // give them the possibility to do whatever they want
                OnNextIteration();


                // make certain we have a callback class
                wxCHECK(GetUnrealCallbacks() != nullptr, -1);
                do
                {
                    if (wxTheApp)
                    {
                        wxTheApp->TickUnreal();
                        wxTheApp->ProcessIdle();
                    }
                } while (!Pending() && !GetUnrealCallbacks()->IsRequestingExit() && !m_shouldExit);

                if (m_shouldExit)
                {
                    while (Pending())
                    {
                        Dispatch();
                    }
                    GetUnrealCallbacks()->SetRequestingExit(true);
                    break;
                }

                while (Pending())
                {
                    if (!Dispatch())
                    {
                        GetUnrealCallbacks()->SetRequestingExit(true);
                        break;
                    }
                }                // give them the possibility to do whatever they want
                OnNextIteration();


                if (GetUnrealCallbacks()->IsRequestingExit() == true)
                {
                    break;
                }
            }

#if wxUSE_EXCEPTIONS
            // exit the outer loop as well
            break;
        }
        catch (...)
        {
            try
            {
                if (!wxTheApp || !wxTheApp->OnExceptionInMainLoop())
                {
                    OnExit();
                    break;
                }
                //else: continue running the event loop
            }
            catch (...)
            {
                // OnException() throwed, possibly rethrowing the same
                // exception again: very good, but we still need OnExit() to
                // be called
                OnExit();
                throw;
            }
        }
    }
#endif // wxUSE_EXCEPTIONS

    return m_exitcode;
}
