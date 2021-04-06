
#include "pch.h"
#include "test_db/database_traits.cpp"

class DB_Relationships : public ::testing::Test
{
public:
   db::cDatabase<cDbTraits> m_db;

   void SetUp() override
   {
   }
};

TEST_F(DB_Relationships, Include)
{
   auto* u1 = (cComp *)m_db.create(eObjId::Comp);
   EXPECT_TRUE(u1);
   EXPECT_TRUE(u1->countPin() == 0);

   auto* p1 = (cPin *)m_db.create(eObjId::Pin);
   EXPECT_TRUE(p1);
   EXPECT_TRUE(p1->countComp() == 0);

   u1->includePin(*p1);
   EXPECT_TRUE(u1->countPin() == 1);
   EXPECT_TRUE(p1->countComp() == 1);

   auto* p2 = (cPin*)m_db.create(eObjId::Pin);
   u1->includePin(*p2);
   EXPECT_TRUE(u1->countPin() == 2);
   EXPECT_TRUE(p1->countComp() == 1);
   EXPECT_TRUE(p2->countComp() == 1);
}
