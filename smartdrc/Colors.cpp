
#include "pch.h"

#include "options_imp.h"

uint32_t cOptionsImp::get_color(int idx)
{
   // color scheme from http://www.festra.com/cb/art-color.htm
   static cColor s_colors[] = {
      cColor(eColor::Aqua, 0, 255, 255),
      cColor(eColor::Black, 0, 0, 0),
      cColor(eColor::Blue, 0, 0, 255),
      cColor(eColor::Cream, 255, 251, 240),
      cColor(eColor::Grey, 128, 128, 128),
      cColor(eColor::Fuchsia, 255, 0, 255),
      cColor(eColor::Green, 0, 128, 0),
      cColor(eColor::Lime,	0, 255, 0),
      cColor(eColor::Maroon, 128, 0, 0),
      cColor(eColor::Navy, 0, 0, 128),
      cColor(eColor::Olive, 128, 128, 0),
      cColor(eColor::Purple, 255, 0, 255),
      cColor(eColor::Red, 255, 0, 0),
      cColor(eColor::Silver, 192, 192, 192),
      cColor(eColor::Teal, 0, 128, 128),
      cColor(eColor::White, 255, 255, 255)
   };

   return s_colors[(int)idx].m_color;
}
