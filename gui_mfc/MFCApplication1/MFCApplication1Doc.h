
// MFCApplication1Doc.h : interface of the CMFCUIDoc class
//


#pragma once
#include "pcb_loader.h"
extern geom::iEngine* GetGeomEngine();

class cDatabase : public iPcbLoaderCallback
{
public:
   DistanceUnit m_unit;
   double m_x1, m_y1, m_x2, m_y2;
   int m_nLayers;

public:
   geom::iEngine* geom_engine() override
   {
      return GetGeomEngine();
   }

   void set_distance_units(DistanceUnit unit) override
   {
      m_unit = unit;
   }
   void set_board_extents(double x1, double y1, double x2, double y2) override
   {
      m_x1 = x1;
      m_x2 = x2;
      m_y1 = y1;
      m_y2 = y2;
   }
   void set_layer_number(int nLayers) override
   {
      m_nLayers = nLayers;
   }
};

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
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
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
