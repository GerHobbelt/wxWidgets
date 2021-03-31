
// blender2dtestView.cpp : implementation of the CBlender2dTestView class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "blender2dtest.h"
#endif

#include "blender2dtestDoc.h"
#include "blender2dtestView.h"
#include "MainFrm.h"

#include "options_imp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

std::vector<BLPoint> _poly;
std::vector<BLPoint> _step;
BLRandom _random(0x1234);
double randomSign() noexcept { return _random.nextDouble() < 0.5 ? 1.0 : -1.0; }

void setPolySize(size_t size, double w, double h) {
   size_t prev = _poly.size();

   _poly.resize(size);
   _step.resize(size);

   while (prev < size) {
      _poly[prev].reset(_random.nextDouble() * w, _random.nextDouble() * h);
      _step[prev].reset((_random.nextDouble() * 0.5 + 0.05) * randomSign(), (_random.nextDouble() * 0.5 + 0.05) * randomSign());
      prev++;
   }
}

void onRenderB2D(BLContext& ctx) noexcept
{
   ctx.setFillStyle(BLRgba32(0xFF000000));
   ctx.fillAll();

   ctx.setFillRule(BL_FILL_RULE_EVEN_ODD);
   ctx.setFillStyle(BLRgba32(0xFFFFFFFF));
   ctx.setStrokeStyle(BLRgba32(0xFFFFFFFF));

   ctx.fillPolygon(_poly.data(), _poly.size());
}

// CBlender2dTestView

IMPLEMENT_DYNCREATE(CBlender2dTestView, CView)

BEGIN_MESSAGE_MAP(CBlender2dTestView, CView)
END_MESSAGE_MAP()

// CBlender2dTestView construction/destruction

CBlender2dTestView::CBlender2dTestView() noexcept
{
	// TODO: add construction code here

}

CBlender2dTestView::~CBlender2dTestView()
{
}

BOOL CBlender2dTestView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CBlender2dTestView drawing

#define BGR(b,g,r) RGB(r,g,b)

void CBlender2dTestView::OnInitialUpdate()
{
   CView::OnInitialUpdate();

   CRect rcClient;
   GetClientRect(&rcClient);

   setPolySize(50, rcClient.Width(), rcClient.Height());
}

void CBlender2dTestView::OnDraw(CDC* pDC)
{
	CBlender2dTestDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

   using namespace chrono;
   auto time_start = steady_clock::now();

   CRect rcClient;
   GetClientRect(&rcClient);

   cBrush brBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

   //++
   auto create_offscreen = [rcClient, &brBackground, pDC]() -> auto {
      cDC memDC = CreateCompatibleDC(*pDC);

      BITMAPINFO bmi{sizeof BITMAPINFO};
      bmi.bmiHeader.biWidth = rcClient.Width();
      bmi.bmiHeader.biHeight = rcClient.Height();
      bmi.bmiHeader.biCompression = BI_RGB;
      bmi.bmiHeader.biBitCount = 32;
      bmi.bmiHeader.biPlanes = 1;

      void* bits;
      cBitmap hOffscreen = CreateDIBSection(*pDC, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
      SelectObject(memDC, hOffscreen);

      return tuple(move(memDC), move(hOffscreen), bmi.bmiHeader.biWidth, bmi.bmiHeader.biHeight, bits);
   };

   auto&& [offscreenDC, offscreenBmp, width, height, bits] = create_offscreen();

   //auto color = [=](int x, int y) -> COLORREF& {
   //   return ((COLORREF*)bits)[y * width + x];
   //};

   //auto red = BGR(255, 0, 0);
   //for (int i = 100; i < 200; ++i) {
   //   color(i, 100) = red;
   //   color(i, 200) = red;
   //}
   m_blImage.reset();
   m_blImage.createFromData(width, height, BL_FORMAT_PRGB32, bits, width * sizeof(COLORREF));

   BLContextCreateInfo createInfo{};
   createInfo.threadCount = 2;

   {
      BLContext ctx(m_blImage, createInfo);
      onRenderB2D(ctx);
   }

   BitBlt(*pDC, 0, 0, width, height, offscreenDC, 0, 0, SRCCOPY);
   //--

   auto time_finish = steady_clock::now();
   auto out_time = [this](const char* msg, auto time) {
      stringstream ss;
      ss << msg << duration_cast<milliseconds>(time).count();
      ss << "ms" << endl;
      if (auto pFrame = (CMainFrame*)GetParentFrame()) {
         pFrame->m_wndStatusBar.SetPaneText(0, CString(ss.str().c_str()));
      }
   };
   out_time("Elapsed: ", time_finish - time_start);
}


// CBlender2dTestView diagnostics

#ifdef _DEBUG
void CBlender2dTestView::AssertValid() const
{
	CView::AssertValid();
}

void CBlender2dTestView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CBlender2dTestDoc* CBlender2dTestView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CBlender2dTestDoc)));
	return (CBlender2dTestDoc*)m_pDocument;
}
#endif //_DEBUG


// CBlender2dTestView message handlers
