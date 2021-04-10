#pragma once
#include "interface.h"

interface iOptions
{
   enum class eColor
   {
      Aqua,
      Black,
      Blue,
      Cream,
      Grey,
      Fuchsia,
      Green,
      Lime,
      Maroon,
      Navy,
      Olive,
      Purple,
      Red,
      Silver,
      Teal,
      White
   };

   virtual COLORREF get_background_color() = 0;
   virtual std::pair<bool, COLORREF> get_visibility(const char* layer, const char* type) = 0;
};
