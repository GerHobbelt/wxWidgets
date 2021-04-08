
#include "pch.h"
#include "test_db.h"
#include "db_string.h"

using namespace std;
using db_string = db::string<char, cDbTraits>;

class StringsGeneral : public ::testing::Test
{
public:
};

NAMESPACE_TEST_F(DB_String, StringsGeneral, Contruction)
{
   const char* sample = "abc";
   {
      db_string s, s1;
      EXPECT_TRUE(s.empty());
      EXPECT_TRUE(s1.empty());
      EXPECT_TRUE(s == s1);
   }
   {
      db_string s = sample;
      EXPECT_TRUE(!s.empty());
      EXPECT_TRUE(s == sample);
      db_string s1 = s;
      EXPECT_TRUE(s1 == s);
      EXPECT_TRUE(!s1.empty());
      EXPECT_TRUE(s1 == sample);
   }
   {
      db_string s(sample);
      EXPECT_TRUE(s == sample);
      db_string s1(move(s));
      EXPECT_TRUE(s.empty());
      EXPECT_TRUE(s1 == sample);
   }
}

NAMESPACE_TEST_F(DB_String, StringsGeneral, Assignment)
{
   const char* sample = "abc";
   db_string s, s1;

   s = sample;
   EXPECT_TRUE(s == sample);
   EXPECT_TRUE(!s.empty());
   EXPECT_TRUE(s.length() == strlen(sample));
   EXPECT_TRUE(s != s1);
   EXPECT_TRUE(s > s1);
   EXPECT_TRUE(s[0] == sample[0]);
   EXPECT_TRUE(s[1] == sample[1]);
   EXPECT_TRUE(s[2] == sample[2]);
   EXPECT_TRUE(s[3] == sample[3]);

   const db_string cs = sample;
   s = cs;
   EXPECT_TRUE(s == cs);
   EXPECT_TRUE(!s.empty());
   EXPECT_TRUE(s.length() == strlen(sample));

   const char* ss = s.c_str();
   EXPECT_TRUE(!strcmp(ss, sample));

   const char* sample2 = "cdef";
   s1 = sample2;
   EXPECT_TRUE(s1 == sample2);
   EXPECT_TRUE(s1.length() == 4);

   s = s1;
   EXPECT_TRUE(s == s1);
   EXPECT_TRUE(s == sample2);
   EXPECT_TRUE(s.length() == strlen(sample2));

   s = move(s1);
   EXPECT_TRUE(s == sample2);
   EXPECT_TRUE(s.length() == strlen(sample2));
   EXPECT_TRUE(s1.empty());
   EXPECT_TRUE(!s.empty());
}

NAMESPACE_TEST_F(DB_String, StringsGeneral, Comparison)
{
   const char *sample = "abc", *sample2 = "ac", *sample3 = "abcd";
   {
      db_string s, s1;
      EXPECT_TRUE(s == s1);
      EXPECT_TRUE(s < sample);
      EXPECT_TRUE(!(s >= sample));
      EXPECT_TRUE(sample > s);
      EXPECT_TRUE(!(sample <= s));
      EXPECT_TRUE(s == "");
      EXPECT_TRUE("" == s);
   }
   {
      db_string s = sample, s1 = sample2;
      EXPECT_TRUE(s == s);
      EXPECT_TRUE(s != s1);
      EXPECT_TRUE(!(s == s1));
      EXPECT_TRUE(s < s1);
      EXPECT_TRUE(!(s >= s1));
   }
   {
      db_string s = sample, s1 = sample3;
      EXPECT_TRUE(s == s);
      EXPECT_TRUE(s != s1);
      EXPECT_TRUE(!(s == s1));
      EXPECT_TRUE(s < s1);
      EXPECT_TRUE(!(s >= s1));
   }
   {
      db_string s = sample;
      EXPECT_TRUE(s == s);
      EXPECT_TRUE(s != sample2);
      EXPECT_TRUE(!(s == sample2));
      EXPECT_TRUE(s < sample2);
      EXPECT_TRUE(!(s >= sample2));
   }
   {
      db_string s = sample;
      EXPECT_TRUE(s == s);
      EXPECT_TRUE(s != sample3);
      EXPECT_TRUE(!(s == sample3));
      EXPECT_TRUE(s < sample3);
      EXPECT_TRUE(!(s >= sample3));
   }
}
