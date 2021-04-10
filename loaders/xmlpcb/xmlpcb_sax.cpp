// pcbxml reader
#include "pch.h"

#include "pcb_loader.h"
#include "database.h"

using namespace std;
using namespace geom;

#undef max
#undef min

#include "../geometry/geom_engine_base.h"

extern cGeomEngineBase* GetGeomEngineBase();

static auto &introspector = cDbTraits::introspector;

using types = db::cTypes<cDbTraits>;
using eObjId = typename types::eObjId;
using cObject = typename types::cObject;

using cChar = char;
const int TEXT_BUFFER_SIZE = 1000;

#define K(x) x
enum class eKeyword {
#include "keywords.h"
};
#undef K

#define K(x) {#x, eKeyword::##x}
static string_map<eKeyword> s_keyword = {
#include "keywords.h"
};
#undef K

#define K(x) x
enum class eObject {
#include "objects.h"
};
#undef K

#define K(x) {#x, eObject::##x}
static string_map<eObject> s_object = {
#include "objects.h"
};
#undef K

class cXmlPcbSaxLoader : public iPcbLoader
{
#define ATT_HANDLER_SIG (eKeyword kw, const cChar* value)

   enum class eShapeType { Unknown = -1, Round, Square, Rectangle, Oval, Finger, Polygon, Path };

#include "loader_base.h"
#include "loader_attribute.h"
#include "loader_area.h"
#include "loader_board_outline.h"
#include "loader_vertex.h"
#include "loader_component.h"
#include "loader_layer.h"
#include "loader_mhole.h"
#include "loader_net.h"
#include "loader_outline.h"
#include "loader_pin.h"
#include "loader_segment.h"
#include "loader_shape.h"
#include "loader_unrouted_segment.h"
#include "loader_via.h"

   static void startElement(void *userData, const char *name, const char **atts)
   {
      auto ldr = (cXmlPcbSaxLoader*)userData;
      if (ldr->m_loader_stack.size()) {
         auto handler = ldr->m_loader_stack.back();
         handler->OnStartElement(name, atts);
      }
      else {
         ldr->OnStartElement(name, atts);
      }
   }

   static void endElement(void *userData, const char *name)
   {
      auto ldr = (cXmlPcbSaxLoader *)userData;
      if (ldr->m_loader_stack.size()) {
         ldr->m_loader_stack.back()->OnEndElement(name);
      }
      else {
         ldr->OnEndElement(name);
      }
   }

   void OnStartElement(const cChar *name, const cChar **atts)
   {
      auto it = s_object.find(name);
      if (it != s_object.end()) {
         switch (it->second) {
            case eObject::BoardOutline:
               m_loader_stack.push_back(new cLoaderBoardOutline(this, atts));
               break;
            case eObject::Layer:
               m_loader_stack.push_back(new cLoaderLayer(this, atts));
               break;
            case eObject::Component:
               m_loader_stack.push_back(new cLoaderComponent(this, atts));
               break;
            case eObject::MountingHole:
               m_loader_stack.push_back(new cLoaderMountingHole(this, atts));
               break;
            case eObject::Net:
               m_loader_stack.push_back(new cLoaderNet(this, atts));
               break;
            default:
               assert(false);
               break;
         }
      }
   }

   void OnEndElement(const cChar *name)
   {
   }

public:
   bool load(const cChar* fname, iPcbLoaderCallback* db) override
   {
      filesystem::path filename = fname;
      if (!exists(filename)) {
         return false;
      }

      m_db = db;
      m_ge = GetGeomEngineBase();

      XML_Parser parser = XML_ParserCreate(nullptr);
      XML_SetElementHandler(parser, startElement, endElement);
      XML_SetUserData(parser, this);

      ifstream inp;
      inp.open(filename, ifstream::in);

      while (true) {
         if (inp.eof()) {
            break;
         }
         char buf[1000];
         memset(buf, 0, sizeof(buf));
         inp.read(buf, sizeof(buf) - 1);
         auto len = (int)strlen(buf);
         if (!XML_Parse(parser, buf, len, len < sizeof(buf) - 1)) {
            return false;
         }
      }

      geom::cRect bounds;
      for (auto &[id, plane]: m_planes) {
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

   iPcbLoaderCallback* m_db = nullptr;

   cGeomEngineBase* m_ge = nullptr;
   map<int, cPlaneBase *> m_planes;
   vector<cLayer *> m_el_layers;

   string_map<list<cMountingHole *>> m_mholes_map;

   vector<cLoaderBase *> m_loader_stack;
};

extern "C" BOOST_SYMBOL_EXPORT iPcbLoader * loader()
{
   return new cXmlPcbSaxLoader;
}
