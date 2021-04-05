#pragma once

#include "smartdrc.h"

#include "options.h"
#include "screen_coord_converter.h"

interface iBitmap
{
   virtual int width() const = 0;
   virtual int height() const = 0;
   virtual uint32_t* colors() = 0;
};

SMARTDRC_API
void DrawBL2D(cDatabase* pDB, iBitmap* pBitmap, const cCoordConverter conv, iOptions* pOptions);
