// pcbxml reader
#include "pch.h"

#include <rapidxml.hpp>

#include "pcb_loader.h"
#include "database.h"

#include "string_utils.h"

using namespace rapidxml;
using namespace std;

static auto& introspector = cDbTraits::introspector;

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

using cChar = char;
const int TEXT_BUFFER_SIZE = 1000;
using cXmlNode = xml_node<cChar>;

class cXmlPcbLoader
   : public iPcbLoader
{
   string_map<cPin*> m_pinmap;
   string_map<cAttributeName*> m_attrmap;

   template <class T>
   auto objmap()
   {
      return (string_map<T*>*)nullptr;
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
      int m_layer = 0, m_index = 0;
      geom::iShape* m_ps = nullptr;
      eShapeType m_shape_type = eShapeType::Unknown;
      geom::ObjectType m_id = geom::ObjectType::unknown;
      geom::coord_t m_diameter = 0, m_width = 0;
      geom::cPoint m_point, m_size, m_seg_begin, m_seg_end;
      bool m_hole = false, m_filled = true, m_closed = false;
      cChar m_text[TEXT_BUFFER_SIZE]{ 0 }, * m_text_end = m_text;
      function<void(cData*)> m_before_creation, m_after_creation;
      geom::iShape* m_current_shape = nullptr;
      geom::iShape* m_current_hole = nullptr;
      list<geom::iShape*> m_shapelist;
      string m_name;

      list<cPin*> m_pinlist;
      list<cVia*> m_vialist;
      list<cMountingHole*> m_mhlist;
      list<cTrace *> m_tracelist;
      list<cAreaFill *> m_arealist;

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
      template <>
      auto &obj_list<cTrace>()
      {
         return m_tracelist;
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
         m_text_end += strlen(m_text_end);
         size_t limit = TEXT_BUFFER_SIZE - (m_text_end - m_text);
         strncpy_s(m_text_end, limit, txt, limit);
      }
      void restore_text()
      {
         *m_text_end = 0;
      }
   };

   geom::iShape* add_to_plane(const cData* data)
   {
      int layer = data->m_layer;
      if (layer && layer <= m_el_layers.size()) {
         if (auto el_layer = m_el_layers[layer - 1]) {
            layer = el_layer->getLayerNumber();
         }
      }
      auto it = m_planes.find(layer);
      if (it == m_planes.end()) {
         it = m_planes.emplace(layer, m_ge->create_plane(layer, data->m_text)).first; // 1-based layer numbering
      }
      if (data->m_ps) {
         geom::iPlane* plane = it->second.get();
         plane->add_shape(data->m_ps, data->m_id);
      }
      return data->m_ps;
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
               data->m_point.m_x = atof(value);
               break;
            case eKeyword::y:
               data->m_point.m_y = atof(value);
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
      data->m_seg_begin = data->m_point;
   }
   OBJ_HANDLER(End)
   {
      loadPoint(node, db, data);
      data->m_seg_end = data->m_point;
   }
   OBJ_HANDLER(Shape)
   {
      loadAttributes(node, data, [this] ATT_HANDLER_SIG {
         switch (attr) {
            case eKeyword::Type:
               data->m_shape_type = (eShapeType)atoi(value);
               break;
            case eKeyword::Void:
               data->m_hole = !!atoi(value);
               break;
            case eKeyword::Filled:
               data->m_filled = !!atoi(value);
               break;
            case eKeyword::Closed:
               data->m_closed = !!atoi(value);
               break;
            case eKeyword::Layer:
               data->m_layer = atoi(value);
               break;
            case eKeyword::Diameter:
               data->m_diameter = atof(value);
               break;
            case eKeyword::Width:
               data->m_width = atof(value);
               break;
            case eKeyword::SizeX:
               data->m_size.m_x = atof(value);
               break;
            case eKeyword::SizeY:
               data->m_size.m_y = atof(value);
               break;
         }
         });

      loadChildNodes(node, db, data);

      if (data->m_before_creation) {
         data->m_before_creation(data);
      }

      if (data->m_ps) {
         data->m_ps = nullptr;
      }
      switch (data->m_shape_type) {
         case eShapeType::Round:
            m_ge->create_circle(&data->m_ps, data->m_point.m_x, data->m_point.m_y, data->m_diameter / 2, data->m_hole, data->m_filled);
            break;
         case eShapeType::Square:
            assert(false); //TBD
            break;
         case eShapeType::Rectangle:
            {
               auto lb = data->m_point - data->m_size / 2, rt = data->m_point + data->m_size / 2;
               m_ge->create_rectangle(&data->m_ps, lb.m_x, lb.m_y, rt.m_x, rt.m_y, data->m_hole, data->m_filled);
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

      if (data->m_after_creation) {
         data->m_after_creation(data);
      }

      if (data->m_ps) {
         data->m_ps->commit();
         add_to_plane(data);
         data->m_shapelist.push_back(data->m_ps);
      }
   }
   OBJ_HANDLER(BoardOutline)
   {
      cData local_data;
      local_data.m_before_creation = [](cData* data) {data->m_filled = false;};

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
      ld.m_layer = layer->getLayerNumber();
      ld.add_text(layer->getName());
      add_to_plane(&ld);
   }
   OBJ_HANDLER(Outline)
   {
      loadChildNodes(node, db, data);
   }
   OBJ_HANDLER(PlacementOutline)
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
      local_data.m_before_creation = [](cData* data) { data->m_filled = false; };
      local_data.add_text(comp->getName());
      local_data.add_text(".");
      loadChildNodes(node, db, &local_data);

      comp->setPosition(local_data.m_point);

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
      obj->setName(data->m_text);
      if (auto pmap = this->objmap<T>()) {
         pmap->emplace(data->m_text, obj);
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
      auto obj = (T *)&*db->create(I);
      data->obj_list<T>().push_back(obj);

      loadAttributes(node, data, [this, obj] ATT_HANDLER_SIG {
         switch (attr) {
            case eKeyword::Name:
               setName(obj, value, data);
               break;
            case eKeyword::NetName:
               data->add_text(value);
               break;
         }
      });

      cData local_data;
      loadChildNodes(node, db, &local_data);

      obj->setPosition(geom::cPoint(local_data.m_point.m_x, local_data.m_point.m_y));

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
      cData ld;
      loadAttributes(node, &ld, [this] ATT_HANDLER_SIG {
         switch (attr) {
            case eKeyword::Name:
               data->m_name = value;
               break;
            case eKeyword::Component:
               data->add_text(value);
               break;
            case eKeyword::Index:
               data->m_index = atoi(value);
               break;
         }
      });

      if (*ld.m_text) {
         ld.add_text(".");
         ld.add_text(ld.m_name.c_str());
         auto it = m_pinmap.find(ld.m_text);
         if (it != m_pinmap.end()) {
            data->m_pinlist.push_back(it->second);
            return;
         }
      }

      loadPads<cPin, cDbTraits::eObjId::Pin>(node, db, data);
   }
   OBJ_HANDLER(Via)
   {
      loadPads<cVia, cDbTraits::eObjId::Via>(node, db, data);
   }
   OBJ_HANDLER(MountingHole)
   {
      cData ld;
      loadPads<cMountingHole, cDbTraits::eObjId::MountingHole>(node, db, &ld);
      if (*ld.m_text) {
         //TBD
      }
   }

   OBJ_HANDLER(Net)
   {
      cData local_data;
      loadChildNodes(node, db, &local_data);

      auto net = db->createNet();

      loadAttributes(node, data, [this, net] ATT_HANDLER_SIG {
         switch (attr) {
            case eKeyword::Name:
               net->setName(value);
               break;
            case eKeyword::NetClass:
               //TBD
               break;
         }
      });

      auto inc = [net](auto& objlist, cDbTraits::eRelId relid) {
         if (auto n = (int)objlist.size()) {
            auto rel = net->get_relationship(relid, false, true);
            rel->resize(n);
            for (auto obj : objlist) {
               net->include(relid, *obj);
            }
         }
      };
      inc(local_data.m_pinlist, cDbTraits::eRelId::Net_Pin);
      inc(local_data.m_vialist, cDbTraits::eRelId::Net_Via);
      inc(local_data.m_tracelist, cDbTraits::eRelId::Net_Trace);
      inc(local_data.m_arealist, cDbTraits::eRelId::Net_AreaFill);
   }

   OBJ_HANDLER(Segment)
   {
      auto trace = db->createTrace();
      data->m_tracelist.push_back(trace);

      cData ld;
      loadAttributes(node, &ld, [this] ATT_HANDLER_SIG {
         switch (attr) {
            case eKeyword::Width:
               data->m_width = atof(value);
               break;
            case eKeyword::Layer:
               data->m_layer = atoi(value);
               break;
         }
      });
      trace->setLayer(ld.m_layer);
      trace->setWidth(ld.m_width);

      loadChildNodes(node, db, &ld);

      trace->setBeg(ld.m_seg_begin);
      trace->setEnd(ld.m_seg_end);

      m_ge->create_segment(&ld.m_ps, ld.m_seg_begin.m_x, ld.m_seg_begin.m_y, ld.m_seg_end.m_x, ld.m_seg_end.m_y, trace->getWidth());
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
   static inline string_map<void(cXmlPcbLoader::*)OBJ_HANDLER_SIG> s_object_handler = {
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
