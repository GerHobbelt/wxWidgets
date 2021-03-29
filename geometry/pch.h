// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"

#define GEOM_ENGINE_IMPL
#include "GeomModel.h"

#include <boost/dll.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;
using namespace geom;

#undef min
#undef max
#include "cavc/polyline.hpp"
#include "cavc/mathutils.hpp"

#endif //PCH_H
