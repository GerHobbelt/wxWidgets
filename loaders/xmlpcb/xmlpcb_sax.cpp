// pcbxml reader
#include "pch.h"

#include "pcb_loader.h"
#include "database.h"
#include "fast_atod.h"

using namespace std;
using namespace geom;

#undef max
#undef min

#include "../geometry/geom_engine_base.h"

extern cGeomEngineBase* GetGeomEngineBase();

using types = db::cTypes<cDbTraits>;
using eObjId = typename types::eObjId;
using cObject = typename types::cObject;

using cChar = char;

constexpr const auto name2int(const char* name)
{
   unsigned retval = 0;
   while (*name) {
      retval = retval * 371 + (unsigned)*name++;
   }
   return retval & 0xFFF;
}

#define K(x) x = name2int(#x),
enum class eKeyword {
#include "keywords.h"
};

enum class eObject {
#include "objects.h"
};
#undef K

class cXmlPcbSaxLoader : public iPcbLoader
{
   enum class eShapeType { Unknown = -1, Round, Square, Rectangle, Oval, Finger, Polygon, Path };

#include "loader_base.h"
#include "loader_attribute.h"
#include "loader_area.h"
#include "loader_board_outline.h"
#include "loader_vertex.h"
#include "loader_cavity.h"
#include "loader_component.h"
#include "loader_drawing.h"
#include "loader_layer.h"
#include "loader_mhole.h"
#include "loader_net.h"
#include "loader_netclass.h"
#include "loader_outline.h"
#include "loader_pin.h"
#include "loader_fiducial.h"
#include "loader_teardrop.h"
#include "loader_region.h"
#include "loader_segment.h"
#include "loader_shape.h"
#include "loader_unrouted_segment.h"
#include "loader_text.h"
#include "loader_via.h"

   static void start_element_static(void *userData, const char *name, const char **atts)
   {
      auto ldr = (cXmlPcbSaxLoader*)userData;
      ldr->start_element(name, atts);
   }

   static void end_element_static(void *userData, const char *name)
   {
      auto ldr = (cXmlPcbSaxLoader *)userData;
      ldr->end_element(name);
   }

   void skip()
   {
      m_skip = m_depth;
   }

   void start_element(const cChar *name, const cChar **atts)
   {
      ++m_depth;
      if (m_skip && m_depth >= m_skip) {
         return;
      }

      if (m_loader_stack.size()) {
         auto handler = m_loader_stack.back();
         handler->OnStartElement(name, atts);
         return;
      }
      switch (auto obj_type = (eObject)name2int(name)) {
#define CASE(n) case eObject::##n##: m_loader_stack.push_back(new cLoader##n##(this, atts)); break;
         CASE(BoardOutline)
         CASE(Layer)
         CASE(Component)
         CASE(MountingHole)
         CASE(Fiducial)
         CASE(Teardrop)
         CASE(Net)
         CASE(NetClass)
         CASE(Region)
         CASE(Cavity)
         CASE(Drawing)
         CASE(Text)
#undef CASE
         default:
            printf("\n");
            break;
      }
   }

   void end_element(const cChar *name)
   {
      if (!m_skip) {
         if (m_loader_stack.size()) {
            m_loader_stack.back()->OnEndElement(name);
         }
      }
      if (m_skip == m_depth) {
         m_skip = 0;
      }

      --m_depth;
   }

   bool read(const cChar* fname)
   {
      XML_Parser parser = XML_ParserCreate(nullptr);
      XML_SetElementHandler(parser, start_element_static, end_element_static);
      XML_SetUserData(parser, this);

      bool retval = false;

      FILE *inp = nullptr;
      if (auto err = fopen_s(&inp, fname, "r"); !err) {
         for (retval = true; !feof(inp);) {
            const int buffer_size = 100000;
            auto buf = (char*)XML_GetBuffer(parser, buffer_size);
            auto len = (int)fread(buf, 1, buffer_size, inp);
            if (!XML_ParseBuffer(parser, len, len < buffer_size)) {
               retval = false;
               break;
            }
         }
         fclose(inp);
      }

      return retval;
   }

public:
   bool load(const cChar* fname, iPcbLoaderCallback* db) override
   {
      m_db = db;
      m_ge = GetGeomEngineBase();

      m_planes[0] = m_ge->create_plane(0, "(All layers)");

      if (!read(fname)) {
         return false;
      }

      for (auto&& [name, attrlist] : m_attrmap) {
         auto it = m_attrnamemap.find(name);
         if (it == m_attrnamemap.end()) {
            auto attrname = m_db->createAttributeName();
            attrname->setName(name.c_str());
            it = m_attrnamemap.emplace(attrname->getName(), attrname).first;
         }
         for (auto& attr : attrlist) {
            it->second->includeAttribute(*attr);
         }
      }

      for (auto&& [netclass_name, netlist] : m_netclassmap) {
         cNetClass* cls = db->createNetClass();
         cls->setName(netclass_name.c_str());
         auto rel = cls->get_relationship(cDbTraits::eRelId::NetClass_Net, false, true);
         auto size = (unsigned)netlist.size();
         rel->resize(size);
         for (auto net : netlist) {
            rel->add(net, cls);
         }
      }

      for (auto &&[nlayer, tracelist]: m_traces_map) {
         cLayer *layer = m_el_layers[nlayer];
         auto rel = layer->get_relationship(cDbTraits::eRelId::Layer_Trace, false, true);
         auto size = (unsigned)tracelist.size();
         rel->resize(size);
         for (auto trace: tracelist) {
            rel->add(trace, layer);
         }
      }

      geom::cRect bounds;
      for (auto& [id, plane] : m_planes) {
         bounds += plane->bounds(); // trigger commit 
      }

      db->set_layer_number(m_planes.size());
      db->set_board_extents(bounds.m_left, bounds.m_bottom, bounds.m_right, bounds.m_top);

      return true;
   }

   void release() override
   {
      delete this;
   }

   cBoardRegion* getRegion(const cChar* name)
   {
      auto it = m_regions_map.find(name);
      if (it != m_regions_map.end()) {
         return it->second;
      }
      auto region = m_db->createBoardRegion();
      m_regions_map[name] = region;
      region->setName(name);
      return region;
   }

   int m_depth = 0, m_skip = 0;

   iPcbLoaderCallback *m_db = nullptr;

   cGeomEngineBase *m_ge = nullptr;
   map<int, cPlaneBase *> m_planes;
   vector<cLayer *> m_layers, m_el_layers;
   map<int, cLayer *> m_metal_layers_map;

   cBoardRegion *m_board = nullptr;

   string_map<list<cMountingHole *>> m_mholes_map;
   string_map<list<cFiducial *>> m_fiducials_map;
   string_map<list<cTeardrop *>> m_teardrops_map;
   string_map<cBoardRegion *> m_regions_map;
   map<int, list<cTrace *>> m_traces_map;

   vector<cLoaderBase *> m_loader_stack;
};

extern "C" BOOST_SYMBOL_EXPORT iPcbLoader * loader()
{
   return new cXmlPcbSaxLoader;
}
