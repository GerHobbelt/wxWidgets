
// MFCApplication1Doc.h : interface of the CMFCUIDoc class
//


#pragma once
#include "pcb_loader.h"
#include "database.h"

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
      return { (int)round(m_db.m_x1), (int)round(m_db.m_y1) };
   }
   CSize GetExtents()
   {
      return { (int)round(m_db.m_x2 - m_db.m_x1), (int)round(m_db.m_y2 - m_db.m_y1) };
   }
   geom::cRect GetWorldRect()
   {
      return geom::cRect(m_db.m_x1, m_db.m_y1, m_db.m_x2, m_db.m_y2);
   }
   geom::iEngine* geom_engine()
   {
      return m_db.geom_engine();
   }
   cDatabase* database()
   {
      return &m_db;
   }

protected:
   cDatabase m_db;

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
