#pragma once

#include "geom_model.h"

enum class DistanceUnit {
   mil, mm
};

interface iPcbLoaderCallback
{
   virtual geom::iEngine * geom_engine() = 0;

   virtual void set_distance_units(DistanceUnit unit) = 0;
   virtual void set_board_extents(double x1, double y1, double x2, double y2) = 0;
   virtual void set_layer_number(int nLayers) = 0;
};
