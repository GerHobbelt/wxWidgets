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

   virtual uint32_t get_background_color() = 0;
   virtual std::pair<bool, uint32_t> get_visibility(const char* layer, int id, const char* type) = 0;
};
