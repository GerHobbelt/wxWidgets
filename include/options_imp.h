#pragma once

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/filesystem.hpp>

namespace pt = boost::property_tree;
namespace fs = boost::filesystem;

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

   virtual std::pair<bool, eColor> get_visibility(const char* layer, const char* type) = 0;
};

struct cOptionsImp
   : public iOptions
{
   pt::ptree options;
   bool loaded = false;

   cOptionsImp(CDocument* pDoc)
   {
      fs::path filename = (LPCTSTR)pDoc->GetPathName();
      filename.replace_extension(".prj");
      if (fs::exists(filename)) {
         pt::read_xml(filename.string(), options);
         loaded = true;
      }
   }
   pt::ptree::path_type layer_key(const char* layer, const char* objtype)
   {
      std::string retval = "options/drawing/";
      retval += layer;
      retval += "/";
      retval += objtype;
      retval += "/<xmlattr>";
      return pt::ptree::path_type(retval, '/');
   }
   std::pair<bool, eColor> get_visibility(const char* layer, const char* type)
   {
      if (loaded) {
         auto path = layer_key(layer, type);
         auto show = options.get<bool>(path / "visible", false);
         auto color = (eColor)options.get<int>(path / "color", 0);
         return { show, color };
      }
      return { true, eColor::Red };
   }
   pt::ptree::path_type view_key(const char* prop)
   {
      pt::ptree::path_type retval("options/viewport", '/');
      retval /= prop;
      retval /= "<xmlattr>/value";
      return retval;
   }
   std::pair<geom::cPoint, double> get_view()
   {
      try {
         pt::ptree::path_type path_scale = view_key("scale");
         pt::ptree::path_type path_x = view_key("center_x");
         pt::ptree::path_type path_y = view_key("center_y");
         geom::cPoint center(options.get<double>(path_x, 0), options.get<double>(path_y, 0));
         auto scale = options.get<double>(path_scale, 0);
         return { center, scale };
      }
      catch (...) {
         return {};
      }
   }
};
