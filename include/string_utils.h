#pragma once

#include <map>

template <class string>
struct string_less
{
   using is_transparent = std::true_type;

   template <typename K>
   bool operator()(const string &s, const K &k) const
   {
      return s.compare(k) < 0;
   }
   template <typename K, typename = std::enable_if_t<!is_same_v<K, string>>>
   bool operator()(const K &k, const string &s) const
   {
      return s.compare(k) > 0;
   }
};

template <typename V, typename C = string_less<std::string>>
using string_map = std::map<std::string, V, C>;
