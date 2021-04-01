#pragma once

#include "pcb_loader_callback.h"

extern geom::iEngine* GetGeomEngine();

class cDatabase : public iPcbLoaderCallback
{
public:
   DistanceUnit m_unit;
   double m_x1, m_y1, m_x2, m_y2;
   int m_nLayers;

public:
   geom::iEngine* geom_engine() override
   {
      return GetGeomEngine();
   }

   void set_distance_units(DistanceUnit unit) override
   {
      m_unit = unit;
   }
   void set_board_extents(double x1, double y1, double x2, double y2) override
   {
      m_x1 = x1;
      m_x2 = x2;
      m_y1 = y1;
      m_y2 = y2;
   }
   void set_layer_number(int nLayers) override
   {
      m_nLayers = nLayers;
   }
};
