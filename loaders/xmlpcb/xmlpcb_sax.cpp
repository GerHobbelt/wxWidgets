// pcbxml reader
#include "pch.h"

#include "pcb_loader.h"
#include "database.h"

using namespace std;

static auto &introspector = cDbTraits::introspector;

using types = db::cTypes<cDbTraits>;
using eObjId = typename types::eObjId;
using cObject = typename types::cObject;

using cChar = char;
const int TEXT_BUFFER_SIZE = 1000;

struct string_less
{
   using is_transparent = true_type;

   template <typename K>
   bool operator()(const string& s, const K& k) const
   {
      return s.compare(k) < 0;
   }
   template <typename K, typename = enable_if_t<!is_same_v<K, string>>>
   bool operator()(const K& k, const string& s) const
   {
      return s.compare(k) > 0;
   }
};

#define K(x) x
enum class eKeyword {
#include "keywords.h"
};
#undef K

#define K(x) {#x, eKeyword::##x}
static map<string, eKeyword, string_less> s_keyword = {
#include "keywords.h"
};
#undef K

#define K(x) x
enum class eObject {
#include "objects.h"
};
#undef K

#define K(x) {#x, eObject::##x}
static map<string, eObject, string_less> s_object = {
#include "objects.h"
};
#undef K

class cXmlPcbSaxLoader : public iPcbLoader
{
#define ATT_HANDLER_SIG (eKeyword kw, const cChar* value)

   enum class eShapeType { Unknown = -1, Round, Square, Rectangle, Oval, Finger, Polygon, Path };

   struct cLoaderBase
   {
      cXmlPcbSaxLoader* m_ldr;

      cLoaderBase(cXmlPcbSaxLoader* ldr)
         : m_ldr(ldr)
      {
      }
      virtual ~cLoaderBase()
      {
      }

      void loadAttributes(const cChar** atts, function<void ATT_HANDLER_SIG> method)
      {
         for (auto att = atts; *att; att += 2) {
            auto it = s_keyword.find(att[0]);
            if (it != s_keyword.end()) {
               auto value = att[1];
               method(it->second, value);
            }
         }
      }

      template <typename P, typename L>
      void include(P& parent, L& objlist, cDbTraits::eRelId relid)
      {
         if (auto n = (int)objlist.size()) {
            auto rel = parent->get_relationship(relid, false, true);
            rel->resize(n);
            for (auto obj: objlist) {
               parent->include(relid, *obj);
            }
         }
      }

      void add_to_plane(geom::iShape *ps, int lay, eObjId type)
      {
         ps->commit();

         auto layer = lay;
         if (layer && layer <= m_ldr->m_el_layers.size()) {
            if (auto el_layer = m_ldr->m_el_layers[layer - 1]) {
               layer = el_layer->getLayerNumber();
            }
         }

         auto it = m_ldr->m_planes.find(layer);
         if (it != m_ldr->m_planes.end()) {
            geom::iPlane *plane = it->second.get();
            plane->add_shape(ps, (geom::ObjectType)type);
         }
      }

      virtual void Delete()
      {
         delete this;
      }

      virtual void OnStartElement(const cChar *name, const cChar **atts) {}
      virtual void OnEndElement(const cChar *name)
      {
         assert(m_ldr->m_loader_stack.back() == this);
         m_ldr->m_loader_stack.pop_back();
         Delete();
      }
   };

   struct cLoaderAttribute : public cLoaderBase
   {
      cLoaderAttribute(cXmlPcbSaxLoader* ldr, const cChar **atts, cAttribute* attr)
         : cLoaderBase(ldr)
      {
         const cChar *name = nullptr, *val = nullptr;
         loadAttributes(atts, [this, &name, &val] ATT_HANDLER_SIG {
            switch (kw) {
               case eKeyword::Name:
                  name = value;
                  break;
               case eKeyword::Value:
                  val = value;
                  break;
            }
         });

         auto it = m_ldr->m_attrmap.find(name);
         if (it == m_ldr->m_attrmap.end()) {
            auto attrname = m_ldr->m_db->createAttributeName();
            attrname->setName(name);
            it = m_ldr->m_attrmap.emplace(attrname->getName(), attrname).first;
         }
         it->second->includeAttribute(*attr);
         attr->setValue(val);
      }
   };

   struct cLoaderArea : public cLoaderBase
   {
      cAreaFill* area = nullptr;
      int layer;

      cLoaderArea(cXmlPcbSaxLoader* ldr, const cChar **atts, cAreaFill* a)
         : cLoaderBase(ldr)
         , area(a)
      {
         loadAttributes(atts, [this] ATT_HANDLER_SIG {
            switch (kw) {
               case eKeyword::Layer:
                  layer = atoi(value);
                  break;
            }
         });
      }
      void OnStartElement(const cChar *name, const cChar **atts) override
      {
         m_ldr->m_loader_stack.push_back(new cLoaderShape(m_ldr, atts, area, eObjId::AreaFill, layer));
      }
   };

   struct cLoaderBoardOutline : public cLoaderBase
   {
      cBoard* board;

      cLoaderBoardOutline(cXmlPcbSaxLoader* ldr, const cChar **atts)
         : cLoaderBase(ldr)
      {
         board = m_ldr->m_db->createBoard();
      }
      void OnStartElement(const cChar *name, const cChar **atts) override
      {
         m_ldr->m_loader_stack.push_back(new cLoaderShape(m_ldr, atts, board, eObjId::Board));
      }
   };

   struct cLoaderVertex : public cLoaderBase
   {
      bool m_arc = false;
      geom::cPoint m_point;
      geom::coord_t m_radius;

      cLoaderVertex()
         : cLoaderBase(nullptr)
      {
      }
      cLoaderVertex(cXmlPcbSaxLoader *ldr, const cChar **atts)
         : cLoaderBase(ldr)
      {
         loadAttributes(atts, [this](eKeyword kw, const cChar *value) {
            switch (kw) {
               case eKeyword::x:
                  m_point.m_x = atof(value);
                  break;
               case eKeyword::y:
                  m_point.m_y = atof(value);
                  break;
               case eKeyword::r:
                  m_arc = true;
                  m_radius = atof(value);
                  break;
            }
         });
      }

      void Delete() override
      {
      }
   };

   struct cLoaderComponent : public cLoaderBase
   {
      cComp *comp;
      list<cPin *> m_pins;
      list<cAttribute *> m_attribs;
      cLoaderVertex position;

      cLoaderComponent(cXmlPcbSaxLoader* ldr, const cChar **atts)
         : cLoaderBase(ldr)
      {
         comp = m_ldr->m_db->createComp();

         loadAttributes(atts, [this] ATT_HANDLER_SIG {
            switch (kw) {
               case eKeyword::Name:
                  comp->setName(value);
                  break;
               case eKeyword::Layer:
                  comp->setLayer(atoi(value));
                  break;
               case eKeyword::PartType:
                  comp->setPartType(value);
                  break;
               case eKeyword::Type: {
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
      }
      void OnStartElement(const cChar *name, const cChar **atts) override
      {
         if (auto it = s_object.find(name); it != s_object.end()) {
            switch (it->second) {
               case eObject::Position: {
                     position = cLoaderVertex(m_ldr, atts);
                     m_ldr->m_loader_stack.push_back(&position);
                     comp->setPosition(position.m_point);
                  } break;
               case eObject::Pin: {
                     cPin * pin = m_ldr->m_db->createPin();
                     m_pins.push_back(pin);
                     m_ldr->m_loader_stack.push_back(new cLoaderPin(m_ldr, atts, comp->getName(), pin));
                  } break;
               case eObject::Attribute: {
                     cAttribute* attr = m_ldr->m_db->createAttribute();
                     m_attribs.push_back(attr);
                     m_ldr->m_loader_stack.push_back(new cLoaderAttribute(m_ldr, atts, attr));
                  } break;
               case eObject::PlacementOutline: {
                     cOutline *ol = m_ldr->m_db->createOutline();
                     comp->includePlacementOutline(*ol);
                     m_ldr->m_loader_stack.push_back(new cLoaderOutline(m_ldr, atts, ol, eObjId::Comp));
                  } break;
               case eObject::Outline: {
                     cOutline *ol = m_ldr->m_db->createOutline();
                     comp->includeOutline(*ol);
                     m_ldr->m_loader_stack.push_back(new cLoaderOutline(m_ldr, atts, ol, eObjId::Comp));
                  } break;
            }
         }
      }
      void OnEndElement(const cChar *name) override
      {
         include(comp, m_pins, cDbTraits::eRelId::Comp_Pin);
         include(comp, m_attribs, cDbTraits::eRelId::Comp_Attribute);

         cLoaderBase::OnEndElement(name);
      }
   };

   struct cLoaderLayer : public cLoaderBase
   {
      cLoaderLayer(cXmlPcbSaxLoader* ldr, const cChar **atts)
         : cLoaderBase(ldr)
      {
         cLayer *layer = ldr->m_db->createLayer();

         loadAttributes(atts, [this, layer] ATT_HANDLER_SIG {
            switch (kw) {
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
                        m_ldr->m_el_layers.push_back(layer);
                        layer->setLayerType(eLayerType::Conductive);
                        layer->setConductiveLayerType(eConductiveLayerType::Signal);
                        break;
                     case 1:
                        layer->setLayerType(eLayerType::Dielectric);
                        layer->setDielectricLayerType(eDielectricLayerType::Substrate);
                        break;
                     case 2:
                        m_ldr->m_el_layers.push_back(layer);
                        layer->setLayerType(eLayerType::Conductive);
                        layer->setConductiveLayerType(eConductiveLayerType::Plane);
                        break;
                     case 3:
                        m_ldr->m_el_layers.push_back(layer);
                        layer->setLayerType(eLayerType::Conductive);
                        layer->setConductiveLayerType(eConductiveLayerType::PlaneNegative);
                        break;
                     case 4:
                        m_ldr->m_el_layers.push_back(layer);
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

         auto n_layer = layer->getLayerNumber();
         auto it = m_ldr->m_planes.find(n_layer);
         if (it == m_ldr->m_planes.end()) {
            m_ldr->m_planes.emplace(n_layer, m_ldr->m_ge->create_plane(n_layer, layer->getName())); // 1-based layer numbering
         }
      }
   };

   struct cLoaderMountingHole : public cLoaderBase
   {
      cMountingHole* mhole = nullptr;
      cLoaderVertex position;
      list<cPad *> m_pads;

      cLoaderMountingHole(cXmlPcbSaxLoader* ldr, const cChar **atts)
         : cLoaderBase(ldr)
      {
         mhole = m_ldr->m_db->createMountingHole();
         loadAttributes(atts, [this] ATT_HANDLER_SIG {
            switch (kw) {
               case eKeyword::Name:
                  mhole->setName(value);
                  break;
               case eKeyword::NetName:
                  m_ldr->m_mholes_map[value].push_back(mhole);
                  break;
               case eKeyword::DrillSize:
                  mhole->setDrillSize(atof(value));
                  break;
               case eKeyword::Plated:
                  mhole->setPlated(!!atoi(value));
                  break;
            }
         });
      }
      void OnStartElement(const cChar *name, const cChar **atts) override
      {
         if (auto it = s_object.find(name); it != s_object.end()) {
            switch (it->second) {
               case eObject::Position: {
                     position = cLoaderVertex(m_ldr, atts);
                     m_ldr->m_loader_stack.push_back(&position);
                     mhole->setPosition(position.m_point);
                  } break;
               case eObject::Shape: {
                     auto pad = m_ldr->m_db->createPad();
                     m_pads.push_back(pad);
                     auto shape = new cLoaderShape(m_ldr, atts, pad, eObjId::MountingHole);
                     pad->setLayer(shape->m_layer);
                     m_ldr->m_loader_stack.push_back(shape);
                  } break;
            }
         }
      }
      void OnEndElement(const cChar *name) override
      {
         include(mhole, m_pads, cDbTraits::eRelId::Object_Pad);
         cLoaderBase::OnEndElement(name);
      }
   };

   struct cLoaderNet : public cLoaderBase
   {
      cNet* net = nullptr;
      list<cPin *> m_pins;
      list<cVia *> m_vias;
      list<cTrace *> m_traces;
      list<cAreaFill *> m_areas;
      list<cAttribute *> m_attrs;
      list<cLink *> m_links;

      cLoaderNet(cXmlPcbSaxLoader* ldr, const cChar **atts)
         : cLoaderBase(ldr)
      {
         net = m_ldr->m_db->createNet();
         loadAttributes(atts, [this] ATT_HANDLER_SIG {
            switch (kw) {
               case eKeyword::Name:
                  net->setName(value);
                  break;
               case eKeyword::NetClass:
                  //TBD
                  break;
            }
         });
      }
      void OnStartElement(const cChar *name, const cChar **atts) override
      {
         if (auto it = s_object.find(name); it != s_object.end()) {
            switch (it->second) {
               case eObject::Pin: {
                     cPin *pin = m_ldr->m_db->createPin();
                     m_pins.push_back(pin);
                     m_ldr->m_loader_stack.push_back(new cLoaderPin(m_ldr, atts, nullptr, pin));
                  } break;
               case eObject::Via: {
                     cVia *via = m_ldr->m_db->createVia();
                     m_vias.push_back(via);
                     m_ldr->m_loader_stack.push_back(new cLoaderVia(m_ldr, atts, via));
                  } break;
               case eObject::Segment: {
                     cTrace* trace = m_ldr->m_db->createTrace();
                     m_traces.push_back(trace);
                     m_ldr->m_loader_stack.push_back(new cLoaderSegment(m_ldr, atts, trace));
                  } break;
               case eObject::Area: {
                     cAreaFill *area = m_ldr->m_db->createAreaFill();
                     m_areas.push_back(area);
                     m_ldr->m_loader_stack.push_back(new cLoaderArea(m_ldr, atts, area));
                  } break;
               case eObject::Attribute: {
                     cAttribute *attr = m_ldr->m_db->createAttribute();
                     m_attrs.push_back(attr);
                     m_ldr->m_loader_stack.push_back(new cLoaderAttribute(m_ldr, atts, attr));
                  } break;
               case eObject::UnroutedSegment: {
                  cLink *link = m_ldr->m_db->createLink();
                  m_links.push_back(link);
                  m_ldr->m_loader_stack.push_back(new cLoaderUnroutedSegment(m_ldr, atts, link));
               } break;
               default:
                  assert(false);
                  break;
            }
         }
      }
      void OnEndElement(const cChar *name) override
      {
         include(net, m_pins, cDbTraits::eRelId::Net_Pin);
         include(net, m_vias, cDbTraits::eRelId::Net_Via);
         include(net, m_traces, cDbTraits::eRelId::Net_Trace);
         include(net, m_areas, cDbTraits::eRelId::Net_AreaFill);
         include(net, m_attrs, cDbTraits::eRelId::Net_Attribute);
         include(net, m_links, cDbTraits::eRelId::Net_Link);

         cLoaderBase::OnEndElement(name);
      }
   };

   struct cLoaderOutline : public cLoaderBase
   {
      cOutline* outline = nullptr;
      eObjId type;

      cLoaderOutline(cXmlPcbSaxLoader* ldr, const cChar **atts, cOutline* ol, eObjId geom_type)
         : cLoaderBase(ldr)
         , outline(ol)
         , type(geom_type)
      {
      }
      void OnStartElement(const cChar *name, const cChar **atts) override
      {
         m_ldr->m_loader_stack.push_back(new cLoaderShape(m_ldr, atts, outline, type));
      }
   };

   struct cLoaderPin : public cLoaderBase
   {
      cPin *pin = nullptr;
      cLoaderVertex position;
      const cChar *name = nullptr;
      const cChar *comp_name = nullptr;
      list<cPad*> m_pads;

      cLoaderPin(cXmlPcbSaxLoader* ldr, const cChar **atts, const cChar* parent_name, cPin* p = nullptr)
         : cLoaderBase(ldr)
         , comp_name(parent_name)
         , pin(p)
      {
         loadAttributes(atts, [this] ATT_HANDLER_SIG {
            switch (kw) {
               case eKeyword::Name:
                  name = value;
                  break;
               case eKeyword::Component:
                  comp_name = value;
                  break;
            }
         });

         const size_t bufsize = 1000;
         cChar buf[bufsize];
         strcpy_s<bufsize>(buf, comp_name);
         strcat_s<bufsize>(buf, ".");
         strcat_s<bufsize>(buf, name);

         if (pin) {
            pin->setName(buf);
            m_ldr->m_pinmap[buf] = pin;
         }
         else {
            auto it = m_ldr->m_pinmap.find(buf);
            if (it != m_ldr->m_pinmap.end()) {
               pin = it->second;
            }
         }
      }
      void OnStartElement(const cChar *name, const cChar **atts) override
      {
         if (auto it = s_object.find(name); it != s_object.end()) {
            switch (it->second) {
               case eObject::Position: {
                     position = cLoaderVertex(m_ldr, atts);
                     m_ldr->m_loader_stack.push_back(&position);
                     pin->setPosition(position.m_point);
                  } break;
               case eObject::Shape: {
                     auto pad = m_ldr->m_db->createPad();
                     m_pads.push_back(pad);
                     auto shape = new cLoaderShape(m_ldr, atts, pad, eObjId::MountingHole);
                     pad->setLayer(shape->m_layer);
                     m_ldr->m_loader_stack.push_back(shape);
                  } break;
            }
         }
      }
      void OnEndElement(const cChar *name) override
      {
         include(pin, m_pads, cDbTraits::eRelId::Object_Pad);
         cLoaderBase::OnEndElement(name);
      }
   };

   struct cLoaderPlacementOutline : public cLoaderBase
   {
      cLoaderPlacementOutline(cXmlPcbSaxLoader* ldr, const cChar **atts)
         : cLoaderBase(ldr)
      {
      }
      void OnStartElement(const cChar *name, const cChar **atts) override
      {
      }
      void OnEndElement(const cChar *name) override
      {
      }
   };

   struct cLoaderPosition : public cLoaderBase
   {
      cLoaderPosition(cXmlPcbSaxLoader* ldr, const cChar **atts)
         : cLoaderBase(ldr)
      {
      }
      void OnStartElement(const cChar *name, const cChar **atts) override
      {
      }
      void OnEndElement(const cChar *name) override
      {
      }
   };

   struct cLoaderSegment : public cLoaderBase
   {
      cTrace* trace = nullptr;
      geom::cPoint beg, end;
      geom::coord_t width;
      int layer;

      cLoaderSegment(cXmlPcbSaxLoader* ldr, const cChar **atts, cTrace* t)
         : cLoaderBase(ldr)
         , trace(t)
      {
         loadAttributes(atts, [this] ATT_HANDLER_SIG{
            switch (kw) {
               case eKeyword::Width:
                  width = atof(value);
                  break;
               case eKeyword::Layer:
                  layer = atoi(value);
                  break;
            }
          });
      }
      void OnStartElement(const cChar *name, const cChar **atts) override
      {
         if (auto it = s_object.find(name); it != s_object.end()) {
            auto v = new cLoaderVertex(m_ldr, atts);
            m_ldr->m_loader_stack.push_back(v);
            switch (it->second) {
               case eObject::Begin:
                  beg = v->m_point;
                  break;
               case eObject::End:
                  end = v->m_point;
                  break;
            }
         }
      }
      void OnEndElement(const cChar *name) override
      {
         geom::iShape* ps = nullptr;
         m_ldr->m_ge->create_segment(&ps, beg.m_x, beg.m_y, end.m_x, end.m_y, width);
         add_to_plane(ps, layer, eObjId::Trace);

         trace->setBeg(beg);
         trace->setEnd(end);
         trace->setLayer(layer);
         trace->setWidth(width);

         cLoaderBase::OnEndElement(name);
      }
   };

   struct cLoaderShape : public cLoaderBase
   {
      eObjId m_type;
      int m_layer = 0;
      cLoaderVertex m_center;
      geom::cPoint m_size;
      geom::iShape *m_ps = nullptr;
      geom::coord_t m_diameter = 0, m_width = 0;
      bool m_hole = false, m_filled = false, m_closed = false, m_prev_arc = false;
      eShapeType m_shape_type = eShapeType::Unknown;
      list<cLoaderVertex> m_vertices;

      cLoaderShape(cXmlPcbSaxLoader* ldr, const cChar **atts, cObject* obj, eObjId type, int l = 0)
         : cLoaderBase(ldr)
         , m_type(type)
         , m_layer(l)
      {
         loadAttributes(atts, [this] ATT_HANDLER_SIG {
            switch (kw) {
               case eKeyword::Type:
                  m_shape_type = (eShapeType)atoi(value);
                  break;
               case eKeyword::Void:
                  m_hole = !!atoi(value);
                  break;
               case eKeyword::Filled:
                  m_filled = !!atoi(value);
                  break;
               case eKeyword::Closed:
                  m_closed = !!atoi(value);
                  break;
               case eKeyword::Layer:
                  m_layer = atoi(value);
                  break;
               case eKeyword::Diameter:
                  m_diameter = atof(value);
                  break;
               case eKeyword::Width:
                  m_width = atof(value);
                  break;
               case eKeyword::SizeX:
                  m_size.m_x = atof(value);
                  break;
               case eKeyword::SizeY:
                  m_size.m_y = atof(value);
                  break;
            }
         });
      }
      void OnStartElement(const cChar *name, const cChar **atts) override
      {
         if (auto it = s_object.find(name); it != s_object.end()) {
            switch (it->second) {
               case eObject::Center:
                  m_center = cLoaderVertex(m_ldr, atts);
                  m_ldr->m_loader_stack.push_back(&m_center);
                  break;
               case eObject::Vertex:
                  m_vertices.emplace_back(m_ldr, atts);
                  m_ldr->m_loader_stack.push_back(&m_vertices.back());
                  break;
            }
         }
      }
      void OnEndElement(const cChar *name) override
      {
         switch (m_shape_type) {
            case eShapeType::Round: {
                  auto pt = m_center.m_point;
                  m_ldr->m_ge->create_circle(&m_ps, pt.m_x, pt.m_y, m_diameter / 2, m_hole, m_filled);
               } break;
            case eShapeType::Square:
            case eShapeType::Rectangle: {
                  auto pt = m_center.m_point;
                  auto lb = pt - m_size / 2, rt = pt + m_size / 2;
                  m_ldr->m_ge->create_rectangle(&m_ps, lb.m_x, lb.m_y, rt.m_x, rt.m_y, m_hole, m_filled);
               } break;
            case eShapeType::Oval:
               assert(false); //TBD
               break;
            case eShapeType::Finger:
               assert(false); //TBD
               break;
            case eShapeType::Path:
            case eShapeType::Polygon:
               m_ldr->m_ge->create_shape(&m_ps, m_hole, m_filled);
               break;
         }
         if (auto size = m_vertices.size()) {
            m_ps->reserve(size);
            geom::coord_t radius;
            geom::cPoint arc_center;
            for (auto& v : m_vertices) {
               if (v.m_arc) {
                  arc_center = v.m_point;
                  m_prev_arc = v.m_arc;
                  radius = v.m_radius;
               }
               else if (m_prev_arc) {
                  m_ps->add_arc(arc_center.m_x, arc_center.m_y, radius, v.m_point.m_x, v.m_point.m_y, true);
                  m_prev_arc = false;
               }
               else {
                  m_ps->add_vertex(v.m_point.m_x, v.m_point.m_y);
               }
            }
         }

         m_ps->commit();

         auto layer = m_layer;
         if (layer && layer <= m_ldr->m_el_layers.size()) {
            if (auto el_layer = m_ldr->m_el_layers[layer - 1]) {
               layer = el_layer->getLayerNumber();
            }
         }

         auto it = m_ldr->m_planes.find(layer);
         if (it != m_ldr->m_planes.end()) {
            geom::iPlane *plane = it->second.get();
            plane->add_shape(m_ps, (geom::ObjectType)m_type);
         }

         m_ps->release();
         cLoaderBase::OnEndElement(name);
      }
   };

   struct cLoaderUnroutedSegment : public cLoaderBase
   {
      cLink* link = nullptr;
      cLoaderVertex beg, end;

      cLoaderUnroutedSegment(cXmlPcbSaxLoader* ldr, const cChar **atts, cLink* l)
         : cLoaderBase(ldr)
         , link(l)
      {
         loadAttributes(atts, [this] ATT_HANDLER_SIG {
            switch (kw) {
               case eKeyword::Layer1:
                  link->setLayer1(atoi(value));
                  break;
               case eKeyword::Layer2:
                  link->setLayer2(atoi(value));
                  break;
            }
         });
      }
      void OnStartElement(const cChar *name, const cChar **atts) override
      {
         if (auto it = s_object.find(name); it != s_object.end()) {
            switch (it->second) {
               case eObject::Begin:
                  beg = cLoaderVertex(m_ldr, atts);
                  m_ldr->m_loader_stack.push_back(&beg);
                  link->setBeg(beg.m_point);
                  break;
               case eObject::End:
                  end = cLoaderVertex(m_ldr, atts);
                  m_ldr->m_loader_stack.push_back(&end);
                  link->setEnd(end.m_point);
                  break;
            }
         }
      }
   };

   struct cLoaderVia : public cLoaderBase
   {
      cVia* via = nullptr;
      cLoaderVertex position;
      list<cPad *> m_pads;

      cLoaderVia(cXmlPcbSaxLoader* ldr, const cChar **atts, cVia* v)
         : cLoaderBase(ldr)
         , via(v)
      {
         loadAttributes(atts, [this] ATT_HANDLER_SIG {
            switch (kw) {
               case eKeyword::DrillSize:
                  via->setDrillSize(atof(value));
                  break;
            }
         });
      }
      void OnStartElement(const cChar *name, const cChar **atts) override
      {
         if (auto it = s_object.find(name); it != s_object.end()) {
            switch (it->second) {
               case eObject::Position: {
                     position =  cLoaderVertex(m_ldr, atts);
                     m_ldr->m_loader_stack.push_back(&position);
                     via->setPosition(position.m_point);
                  } break;
               case eObject::Shape: {
                     auto pad = m_ldr->m_db->createPad();
                     m_pads.push_back(pad);
                     auto shape = new cLoaderShape(m_ldr, atts, pad, eObjId::MountingHole);
                     pad->setLayer(shape->m_layer);
                     m_ldr->m_loader_stack.push_back(shape);
                  } break;
            }
         }
      }
      void OnEndElement(const cChar *name) override
      {
         include(via, m_pads, cDbTraits::eRelId::Object_Pad);
         cLoaderBase::OnEndElement(name);
      }
   };

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
      m_ge = db->geom_engine();

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

//#define K(x) {#x, [](cXmlPcbSaxLoader* ldr, const cChar** atts) -> cLoaderBase* {return new cLoader##x(ldr, atts);}}
//   static inline map<string, function<cLoaderBase* (cXmlPcbSaxLoader* ldr, const cChar** atts)>, string_less> s_object_handler = {
//   #include "objects.h"
//   };
//#undef K

   iPcbLoaderCallback* m_db = nullptr;
   geom::iEngine* m_ge = nullptr;
   vector<cLoaderBase *> m_loader_stack;
   map<int, unique_ptr<geom::iPlane>> m_planes;
   vector<cLayer*> m_el_layers;
   map<string, cPin*, string_less> m_pinmap;
   map<string, cAttributeName *, string_less> m_attrmap;
   map<string, list<cMountingHole*>, string_less> m_mholes_map;
};

extern "C" BOOST_SYMBOL_EXPORT iPcbLoader * loader()
{
   return new cXmlPcbSaxLoader;
}
