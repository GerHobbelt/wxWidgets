
#include "pch.h"

#ifdef _DEBUG
  #pragma comment(lib, "wxmsw31ud_aui.lib")
  #pragma comment(lib, "wxmsw31ud_html.lib")
  #pragma comment(lib, "wxmsw31ud_core.lib")
  #pragma comment(lib, "wxbase31ud_xml.lib")
  #pragma comment(lib, "wxbase31ud.lib")
  #pragma comment(lib, "wxtiffd.lib")
  #pragma comment(lib, "wxjpegd.lib")
  #pragma comment(lib, "wxpngd.lib")
  #pragma comment(lib, "wxzlibd.lib")
  #pragma comment(lib, "wxregexud.lib")
  #pragma comment(lib, "wxexpatd.lib")
#else
   #pragma comment(lib, "wxmsw31u_aui.lib")
   #pragma comment(lib, "wxmsw31u_html.lib")
   #pragma comment(lib, "wxmsw31u_core.lib")
   #pragma comment(lib, "wxbase31u_xml.lib")
   #pragma comment(lib, "wxbase31u.lib")
   #pragma comment(lib, "wxtiff.lib")
   #pragma comment(lib, "wxjpeg.lib")
   #pragma comment(lib, "wxpng.lib")
   #pragma comment(lib, "wxzlib.lib")
   #pragma comment(lib, "wxregexu.lib")
   #pragma comment(lib, "wxexpat.lib")
#endif
