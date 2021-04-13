
// MFCApplication1Doc.h : interface of the CMFCUIDoc class
//


#pragma once
#include "pcb_loader.h"
#include "smartdrc.h"

class CMFCUIDoc : public CDocument
{
protected: // create from serialization only
	CMFCUIDoc() noexcept;
	DECLARE_DYNCREATE(CMFCUIDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	BOOL OnNewDocument() override;
	void Serialize(CArchive& ar) override;
   void DeleteContents() override; // delete doc items etc

#ifdef SHARED_HANDLERS
	void InitializeSearchContent() override;
	void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds) override;
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CMFCUIDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

   CPoint GetOrigin()
   {
      auto db = database();
      return {(int)round(db->m_x1), (int)round(db->m_y1)};
   }
   CSize GetExtents()
   {
      auto db = database();
      return {(int)round(db->m_x2 - db->m_x1), (int)round(db->m_y2 - db->m_y1)};
   }
   geom::cRect GetWorldRect()
   {
      auto db = database();
      return geom::cRect(db->m_x1, db->m_y1, db->m_x2, db->m_y2);
   }
   cDatabase* database()
   {
      return m_db ? m_db->database() : nullptr;
   }

protected:
   std::unique_ptr<iDbHolder> m_db;

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
