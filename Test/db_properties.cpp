
#include "pch.h"
#include "test_db/database.h"
#include "test_db/database_traits_types.h"

class DB_Properties : public ::testing::Test
{
public:
   db::cDatabase<cDbTraits> m_db;

   void SetUp() override
   {
   }
};

TEST_F(DB_Properties, Set)
{
   auto* u1 = (cComp*)m_db.create(eObjId::Comp);
   EXPECT_TRUE(u1);
}
