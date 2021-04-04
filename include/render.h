#pragma once

#include "smartdrc.h"

#include "options.h"
#include "screen_coord_converter.h"

interface iBitmap;

SMARTDRC_API
void DrawBL2D(cDatabase* pDB, iBitmap* pBitmap, const cCoordConverter conv, iOptions* pOptions);
