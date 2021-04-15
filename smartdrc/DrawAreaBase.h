#pragma once

#include <filesystem>

#include "options_imp.h"
#include "screen_coord_converter.h"

struct SMARTDRC_API cDrawAreaBase
{
   using coord_t = cCoordConverter::coord_t;
   using cScreenRect = cCoordConverter::cScreenRect;
   using cScreenPoint = cCoordConverter::cScreenPoint;
   using cScreenUpdateDesc = cCoordConverter::cScreenUpdateDesc;

   cCoordConverter m_conv;
   cScreenPoint m_scroll_size;
   std::unique_ptr<cOptionsImp> m_cvd;

   void OnRestoreView(cDatabase *db, const cScreenRect& rc, const std::filesystem::path& project);

   virtual void UpdateScrollBars(bool bRedraw = true) = 0;
};
