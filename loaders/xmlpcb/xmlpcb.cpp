// pcbxml reader
#include "pch.h"

#include <rapidxml.hpp>

#include "pcb_loader.h"
#include "database.h"

using namespace rapidxml;
using namespace std;

auto& introspector = cDbTraits::introspector;

struct string_less
{
   bool operator()(const char* s, const char* k) const
   {
      return strcmp(s, k) < 0;
   }
};

#define K(x) x
enum class eKeyword {
#include "keywords.h"
};
#undef K

#define K(x) {#x, eKeyword::##x}
static map<const char*, eKeyword, string_less> s_keyword = {
#include "keywords.h"
};
#undef K

using cChar = char;
const int TEXT_BUFFER_SIZE = 1000;
using cXmlNode = xml_node<cChar>;

class cXmlPcbLoader
   : public iPcbLoader
{
   map<const char*, cPin*, string_less> m_pinmap;

   template <class T>
   auto objmap()
   {
      return (map<const char*, T*, string_less>*)nullptr;
   }
   template <>
   auto objmap<cPin>()
   {
      return &m_pinmap;
   }

public:
   bool load(const char* fname, iPcbLoaderCallback* db) override
   {
      filesystem::path filename = fname;
      if (!exists(filename)) {
         return false;
      }

      m_ge = db->geom_engine();

      size_t fsize = file_size(filename);
      vector<char> file_contents(fsize + 1);
      ifstream inp;
      inp.open(filename, ifstream::in);
      auto ss = &file_contents.front();
      inp.read(ss, fsize);
      file_contents[fsize] = 0;

      xml_document<cChar> doc;
      doc.parse<0>(file_contents.data());

      loadChildNodes(doc.first_node(), db, nullptr);

      geom::cRect bounds;
      for (auto& [id, plane]: m_planes) {
         bounds += plane->bounds(); // trigger commit
      }

      db->set_layer_number(m_planes.size());
      db->set_board_extents(bounds.m_left, bounds.m_bottom, bounds.m_right, bounds.m_top);

      doc.clear();
      return true;
   }
   void release() override
   {
      delete this;
   }

   enum class eShapeType { Unknown = -1, Round, Square, Rectangle, Oval, Finger, Polygon, Path };

   struct cData
   {
      int layer = 0;
      geom::iShape* ps = nullptr;
      eShapeType shape_type = eShapeType::Unknown;
      geom::ObjectType id = geom::ObjectType::unknown;
      geom::coord_t diameter = 0, width = 0;
      geom::cPoint point, size, seg_begin, seg_end;
      bool hole = false, filled = true, closed = false;
      cChar text[TEXT_BUFFER_SIZE]{ 0 }, * text_end = text;
      function<void(cData*)> before_creation, after_creation;
      geom::iShape* m_current_shape = nullptr;
      geom::iShape* m_current_hole = nullptr;
      list<geom::iShape*> m_shapelist;

      list<cPin*> m_pinlist;
      list<cVia*> m_vialist;
      list<cMountingHole*> m_mhlist;

      template <typename T>
      auto& obj_list()
      {
         return list<void*>();
      }
      template <>
      auto& obj_list<cPin>()
      {
         return m_pinlist;
      }
      template <>
      auto& obj_list<cVia>()
      {
         return m_vialist;
      }
      template <>
      auto& obj_list<cMountingHole>()
      {
         return m_mhlist;
      }

      ~cData()
      {
         if (m_current_hole) {
            m_current_hole->commit();
         }
         if (m_current_shape) {
            m_current_shape->commit();
         }
      }

      void add_text(const cChar* txt)
      {
         text_end += strlen(text_end);
         size_t limit = TEXT_BUFFER_SIZE - (text_end - text);
         strncpy_s(text_end, limit, txt, limit);
      }
      void restore_text()
      {
         *text_end = 0;
      }
   };

   geom::iShape* add_to_plane(const cData* data)
   {
      int layer = data->layer;
      if (layer && layer <= m_el_layers.size()) {
         if (auto el_layer = m_el_layers[layer - 1]) {
            layer = el_layer->getLayerNumber();
         }
      }
      auto it = m_planes.find(layer);
      if (it == m_planes.end()) {
         it = m_planes.emplace(layer, m_ge->create_plane(layer, data->text)).first; // 1-based layer numbering
      }
      if (data->ps) {
         geom::iPlane* plane = it->second.get();
         plane->add_shape(data->ps, data->id);
      }
      return data->ps;
   }

#define ATT_HANDLER_SIG (eKeyword attr, const cChar* value, cData* data)
#define OBJ_HANDLER_SIG (cXmlNode * node, iPcbLoaderCallback * db, cData * data)
#define OBJ_HANDLER(x) void load##x OBJ_HANDLER_SIG

   OBJ_HANDLER(ChildNodes)
   {
      for (auto n = node->first_node(); n; n = n->next_sibling()) {
         auto it = s_object_handler.find(n->name());
         if (it != s_object_handler.end()) {
            (this->*(it->second))(n, db, data);
            continue;
         }
         assert(false);
      }
   }
   void loadAttributes(cXmlNode* node, cData* data, function<void(eKeyword, const cChar *, cData* data)> method)
   {
      for (auto attr = node->first_attribute(); attr; attr = attr->next_attribute()) {
         auto it = s_keyword.find(attr->name());
         if (it != s_keyword.end()) {
            auto value = attr->value();
            method(it->second, value, data);
         }
      }
   }

   OBJ_HANDLER(Point)
   {
      loadAttributes(node, data, [this] ATT_HANDLER_SIG {
         switch (attr) {
            case eKeyword::x:
               data->point.m_x = atof(value);
               break;
            case eKeyword::y:
               data->point.m_y = atof(value);
               break;
         }
      });
   }
   OBJ_HANDLER(Center)
   {
      loadPoint(node, db, data);
   }
   OBJ_HANDLER(Vertex)
   {
      loadPoint(node, db, data);
   }
   OBJ_HANDLER(Position)
   {
      loadPoint(node, db, data);
   }
   OBJ_HANDLER(Begin)
   {
      loadPoint(node, db, data);
      data->seg_begin = data->point;
   }
   OBJ_HANDLER(End)
   {
      loadPoint(node, db, data);
      data->seg_end = data->point;
   }
   OBJ_HANDLER(Shape)
   {
      loadAttributes(node, data, [this] ATT_HANDLER_SIG {
         switch (attr) {
            case eKeyword::Type:
               data->shape_type = (eShapeType)atoi(value);
               break;
            case eKeyword::Void:
               data->hole = !!atoi(value);
               break;
            case eKeyword::Filled:
               data->filled = !!atoi(value);
               break;
            case eKeyword::Closed:
               data->closed = !!atoi(value);
               break;
            case eKeyword::Layer:
               data->layer = atoi(value);
               break;
            case eKeyword::Diameter:
               data->diameter = atof(value);
               break;
            case eKeyword::Width:
               data->width = atof(value);
               break;
            case eKeyword::SizeX:
               data->size.m_x = atof(value);
               break;
            case eKeyword::SizeY:
               data->size.m_y = atof(value);
               break;
         }
         });

      loadChildNodes(node, db, data);

      if (data->before_creation) {
         data->before_creation(data);
      }

      if (data->ps) {
         data->ps = nullptr;
      }
      switch (data->shape_type) {
         case eShapeType::Round:
            m_ge->create_circle(&data->ps, data->point.m_x, data->point.m_y, data->diameter / 2, data->hole, data->filled);
            break;
         case eShapeType::Square:
            assert(false); //TBD
            break;
         case eShapeType::Rectangle:
            {
               auto lb = data->point - data->size / 2, rt = data->point + data->size / 2;
               m_ge->create_rectangle(&data->ps, lb.m_x, lb.m_y, rt.m_x, rt.m_y, data->hole, data->filled);
            } break;
         case eShapeType::Oval:
            assert(false); //TBD
            break;
         case eShapeType::Finger:
            assert(false); //TBD
            break;
         case eShapeType::Polygon:
            break;
         case eShapeType::Path:
            break;
      }

      if (data->after_creation) {
         data->after_creation(data);
      }

      if (data->ps) {
         data->ps->commit();
         add_to_plane(data);
         data->m_shapelist.push_back(data->ps);
      }
   }
   OBJ_HANDLER(BoardOutline)
   {
      cData local_data;
      local_data.before_creation = [](cData* data) {data->filled = false;};

      auto pBoard = db->createBoard();

      loadChildNodes(node, db, &local_data);
   }
   OBJ_HANDLER(Layer)
   {
      cLayer* layer = db->createLayer();

      loadAttributes(node, data, [this, layer] ATT_HANDLER_SIG {
         switch (attr) {
            case eKeyword::Name:
               layer->setName(value);
               break;
            case eKeyword::Number:
               layer->setLayerNumber(atoi(value) + 1); // 1-based
               break;
            case eKeyword::Thickness:
               layer->setThickness(atof(value));
               break;
            case eKeyword::Dielectric:
               layer->setDielectricConstant(atof(value));
               break;
            case eKeyword::Type: {
               switch (int type = atoi(value)) {
                  case 0:
                     m_el_layers.push_back(layer);
                     layer->setLayerType(eLayerType::Conductive);
                     layer->setConductiveLayerType(eConductiveLayerType::Signal);
                     break;
                  case 1:
                     layer->setLayerType(eLayerType::Dielectric);
                     layer->setDielectricLayerType(eDielectricLayerType::Substrate);
                     break;
                  case 2:
                     m_el_layers.push_back(layer);
                     layer->setLayerType(eLayerType::Conductive);
                     layer->setConductiveLayerType(eConductiveLayerType::Plane);
                     break;
                  case 3:
                     m_el_layers.push_back(layer);
                     layer->setLayerType(eLayerType::Conductive);
                     layer->setConductiveLayerType(eConductiveLayerType::PlaneNegative);
                     break;
                  case 4:
                     m_el_layers.push_back(layer);
                     layer->setLayerType(eLayerType::Conductive);
                     layer->setConductiveLayerType(eConductiveLayerType::Split);
                     break;
                  case 5:
                     layer->setLayerType(eLayerType::Dielectric);
                     layer->setManufacturingLayerType(eManufacturingLayerType::SilkscreenTop);
                     break;
                  case 6:
                     layer->setLayerType(eLayerType::Dielectric);
                     layer->setManufacturingLayerType(eManufacturingLayerType::SilkscreenBottom);
                     break;
                  case 7:
                     layer->setLayerType(eLayerType::Dielectric);
                     layer->setDielectricLayerType(eDielectricLayerType::SolderMaskTop);
                     break;
                  case 8:
                     layer->setLayerType(eLayerType::Dielectric);
                     layer->setDielectricLayerType(eDielectricLayerType::SolderMaskBottom);
                     break;
                  case 9:
                     layer->setLayerType(eLayerType::Dielectric);
                     layer->setManufacturingLayerType(eManufacturingLayerType::SolderPasteTop);
                     break;
                  case 10:
                     layer->setLayerType(eLayerType::Dielectric);
                     layer->setManufacturingLayerType(eManufacturingLayerType::SolderPasteBottom);
                     break;
                  default:
                     layer->setLayerType(eLayerType::Unknown);
                     break;
               }
            } break;
         }
      });

      cData ld;
      ld.layer = layer->getLayerNumber();
      ld.add_text(layer->getName());
      add_to_plane(&ld);
   }
   OBJ_HANDLER(Outline)
   {
      loadChildNodes(node, db, data);
   }
   OBJ_HANDLER(Component)
   {
      cComp* comp = db->createComp();

      loadAttributes(node, data, [this, comp] ATT_HANDLER_SIG {
            switch (attr) {
               case eKeyword::Name:
                  comp->setName(value);
                  break;
               case eKeyword::Layer:
                  comp->setLayer(atoi(value));
                  break;
               case eKeyword::PartType:
                  comp->setPartType(value);
                  break;
               case eKeyword::Type:
                  {
                     int type = atoi(value);
                     if (type < 7) {
                        ++type;
                     }
                     else if (type == 7) {
                        type = 0;
                     }
                     comp->setCompType((eCompType::value)type);
                  } break;
            }
      });

      cData local_data;
      local_data.before_creation = [](cData* data) { data->filled = false; };
      local_data.add_text(comp->getName());
      local_data.add_text(".");
      loadChildNodes(node, db, &local_data);

      comp->setPosition(local_data.point);

      //TBD outlines

      auto n_pins = (int)local_data.m_pinlist.size();
      auto rel = comp->get_relationship(cDbTraits::eRelId::Comp_Pin, false, true);
      rel->resize(n_pins);
      for (auto pin : local_data.m_pinlist) {
         comp->includePin(*pin);
      }
   }

   template <class T>
   void setName(T * obj, const cChar* value, cData* data)
   {
      data->add_text(value);
      obj->setName(data->text);
      if (auto pmap = this->objmap<T>()) {
         pmap->emplace(data->text, obj);
      }
      data->restore_text();
   }
   template <>
   void setName(cVia* obj, const cChar* value, cData* data)
   {
   }

   template <class T, cDbTraits::eObjId I>
   void loadPads OBJ_HANDLER_SIG
   {
      auto obj = (T *)db->create(I);
      data->obj_list<T>().push_back(obj);

      loadAttributes(node, data, [this, obj] ATT_HANDLER_SIG {
         switch (attr) {
            case eKeyword::Name:
               setName(obj, value, data);
               break;
         }
      });

      cData local_data;
      loadChildNodes(node, db, &local_data);

      obj->setPosition(geom::cPoint(local_data.point.m_x, local_data.point.m_y));

      auto n_pads = (int)local_data.m_shapelist.size();
      auto rel = obj->get_relationship(cDbTraits::eRelId::Object_Pad, false, true);
      rel->resize(n_pads);
      for (auto pad_shape: local_data.m_shapelist) {
         auto pad = db->createPad();
         //TBD shape
         obj->includePad(*pad);
      }
   }

   OBJ_HANDLER(Attribute)
   {
      //TBD
   }

   OBJ_HANDLER(Pin)
   {
      loadPads<cPin, cDbTraits::eObjId::Pin>(node, db, data);
   }
   OBJ_HANDLER(Via)
   {
      loadPads<cVia, cDbTraits::eObjId::Via>(node, db, data);
   }
   OBJ_HANDLER(MountingHole)
   {
      cData local_data;
      loadPads<cMountingHole, cDbTraits::eObjId::MountingHole>(node, db, &local_data);
   }

   OBJ_HANDLER(Net)
   {
      cData local_data;
      loadChildNodes(node, db, &local_data);
   }

   OBJ_HANDLER(Segment)
   {
      auto trace = db->createTrace();

      cData ld;
      loadAttributes(node, &ld, [this] ATT_HANDLER_SIG {
         switch (attr) {
            case eKeyword::Width:
               data->width = atof(value);
               break;
            case eKeyword::Layer:
               data->layer = atoi(value);
               break;
         }
      });
      trace->setLayer(ld.layer);
      trace->setWidth(ld.width);

      loadChildNodes(node, db, &ld);

      trace->setBeg(ld.seg_begin);
      trace->setEnd(ld.seg_end);

      m_ge->create_segment(&ld.ps, ld.seg_begin.m_x, ld.seg_begin.m_y, ld.seg_end.m_x, ld.seg_end.m_y, trace->getWidth());
      add_to_plane(&ld);
   }

   OBJ_HANDLER(UnroutedSegment)
   {
      //TBD
   }

   OBJ_HANDLER(Area)
   {
      //TBD
   }

#define K(x) {#x, &cXmlPcbLoader::load##x}
   static inline map<const char*, void(cXmlPcbLoader::*)OBJ_HANDLER_SIG, string_less> s_object_handler = {
   #include "objects.h"
   };
#undef K
   geom::iEngine* m_ge = nullptr;
   map<int, unique_ptr<geom::iPlane>> m_planes;
   vector<cLayer*> m_el_layers;
};

extern "C" BOOST_SYMBOL_EXPORT
iPcbLoader * loader()
{
   return new cXmlPcbLoader;
}
