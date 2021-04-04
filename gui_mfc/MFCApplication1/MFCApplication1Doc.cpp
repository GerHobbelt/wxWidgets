
// MFCApplication1Doc.cpp : implementation of the CMFCUIDoc class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "MFCApplication1.h"
#endif

#include "MFCApplication1Doc.h"

#include <propkey.h>

#include <smartdrc.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMFCUIDoc

IMPLEMENT_DYNCREATE(CMFCUIDoc, CDocument)

BEGIN_MESSAGE_MAP(CMFCUIDoc, CDocument)
END_MESSAGE_MAP()


// CMFCUIDoc construction/destruction

CMFCUIDoc::CMFCUIDoc() noexcept
{
	// TODO: add one-time construction code here

}

CMFCUIDoc::~CMFCUIDoc()
{
}

BOOL CMFCUIDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CMFCUIDoc serialization

void CMFCUIDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) {
		// TODO: add storing code here
	}
	else {
      try {
               ar.GetFile()->Close();

			load_design((LPCTSTR)ar.m_strFileName, &m_db);
      
               ar.GetFile()->Open(ar.m_strFileName, CFile::modeRead);
            }
      catch (...) {
         ASSERT(false);
         int i = 0;
      }
	}
}

void CMFCUIDoc::DeleteContents()
{
   geom::iEngine* ge = geom_engine();
	ge->clear();
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CMFCUIDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CMFCUIDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data.
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CMFCUIDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = nullptr;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != nullptr)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CMFCUIDoc diagnostics

#ifdef _DEBUG
void CMFCUIDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMFCUIDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CMFCUIDoc commands
