//
// pch.h
// Header for standard system include files.
//

#pragma once

#include "gtest/gtest.h"
#include <boost/dll.hpp>
#include "db_relationship.h"

inline bool fuzzy_eq(double x, double y)
{
   auto d = std::max(std::abs(x), std::abs(y));
   if (d == 0.0) {
      return true;
   }
   return std::abs(x - y) / d < 1e-6;
}

#define EXPECT_DEQ(a, b) EXPECT_TRUE(fuzzy_eq(a, b))

#define EXPECT_RECT_EQ(a, b)           \
   EXPECT_DEQ(a.m_left, b.m_left);     \
   EXPECT_DEQ(a.m_right, b.m_right);   \
   EXPECT_DEQ(a.m_top, b.m_top);       \
   EXPECT_DEQ(a.m_bottom, b.m_bottom);
