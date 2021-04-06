
#include "pch.h"
#include "test_db.h"

class Many2Many : public DB_Test
{
public:
};

NAMESPACE_TEST_F(DB_Relationships, Many2Many, IncludeExclude)
{
   u2 = create_comp("U2");
   u1->includeModelSelector(*s1);
   EXPECT_TRUE(u1->countModelSelectors() == 1);
   EXPECT_TRUE(u2->countModelSelectors() == 0);
   EXPECT_TRUE(s1->countComps() == 1);
   EXPECT_TRUE(s2->countComps() == 0);

   u1->includeModelSelector(*s2);
   EXPECT_TRUE(u1->countModelSelectors() == 2);
   EXPECT_TRUE(u2->countModelSelectors() == 0);
   EXPECT_TRUE(s1->countComps() == 1);
   EXPECT_TRUE(s2->countComps() == 1);

   u1->excludeModelSelector(*s2);
   EXPECT_TRUE(u1->countModelSelectors() == 1);
   EXPECT_TRUE(u2->countModelSelectors() == 0);
   EXPECT_TRUE(s1->countComps() == 1);
   EXPECT_TRUE(s2->countComps() == 0);

   u1->includeModelSelector(*s2);
   EXPECT_TRUE(u1->countModelSelectors() == 2);
   EXPECT_TRUE(u2->countModelSelectors() == 0);
   EXPECT_TRUE(s1->countComps() == 1);
   EXPECT_TRUE(s2->countComps() == 1);

   u1->excludeModelSelector(*s1);
   EXPECT_TRUE(u1->countModelSelectors() == 1);
   EXPECT_TRUE(u2->countModelSelectors() == 0);
   EXPECT_TRUE(s1->countComps() == 0);
   EXPECT_TRUE(s2->countComps() == 1);

   u1->excludeModelSelector(*s2);
   EXPECT_TRUE(u1->countModelSelectors() == 0);
   EXPECT_TRUE(u2->countModelSelectors() == 0);
   EXPECT_TRUE(s1->countComps() == 0);
   EXPECT_TRUE(s2->countComps() == 0);

   u1->excludeModelSelector(*s2);
   EXPECT_TRUE(u1->countModelSelectors() == 0);
   EXPECT_TRUE(u2->countModelSelectors() == 0);
   EXPECT_TRUE(s1->countComps() == 0);
   EXPECT_TRUE(s2->countComps() == 0);
}

NAMESPACE_TEST_F(DB_Relationships, Many2Many, Iteration)
{
   names nn = get_names(u1->ModelSelectors());
   EXPECT_TRUE(names_eq(nn, {}));

   u1->includeModelSelector(*s1);
   u1->includeModelSelector(*s2);

   nn = get_names(u1->ModelSelectors());
   EXPECT_TRUE(names_eq(nn, { "S1", "S2" }));

   nn.clear();
   auto&& r = u1->ModelSelectors();
   for (auto isel = r.begin(); isel != r.end(); ++isel) {
      nn.push_back(isel->m_Name);
   }
   EXPECT_TRUE(names_eq(nn, { "S1", "S2" }));

   nn.clear();
   for (auto& isel : u1->ModelSelectors()) {
      nn.push_back(isel.m_Name);
      if (isel.m_Name == "S2") {
         u1->includeModelSelector(*s3);
      }
   }
   EXPECT_TRUE(names_eq(nn, { "S1", "S2", "S3" }));

   u1->excludeModelSelector(*s1);
   nn = get_names(u1->ModelSelectors());
   EXPECT_TRUE(names_eq(nn, {"S2", "S3"}));

   u1->excludeModelSelector(*s2);
   nn = get_names(u1->ModelSelectors());
   EXPECT_TRUE(names_eq(nn, {"S3"}));

   u1->excludeModelSelector(*s3);
   nn = get_names(u1->ModelSelectors());
   EXPECT_TRUE(names_eq(nn, {}));

   u1->includeModelSelector(*s1);
   u1->includeModelSelector(*s2);
   u1->includeModelSelector(*s3);

   nn = get_names(u1->ModelSelectors());
   EXPECT_TRUE(names_eq(nn, {"S3", "S2", "S1"}));

   u1->excludeModelSelector(*s3);
   nn = get_names(u1->ModelSelectors());
   EXPECT_TRUE(names_eq(nn, {"S2", "S1"}));
}

NAMESPACE_TEST_F(DB_Relationships, Many2Many, Deletion)
{
   u1->includeModelSelector(*s1);
   u1->includeModelSelector(*s2);
   u1->includeModelSelector(*s3);

   names nn = get_names(u1->ModelSelectors());
   EXPECT_TRUE(names_eq(nn, {"S1", "S2", "S3" }));

   m_db.erase(s1);

   nn = get_names(u1->ModelSelectors());
   EXPECT_TRUE(names_eq(nn, {"S2", "S3"}));

   m_db.erase(s2);

   nn = get_names(u1->ModelSelectors());
   EXPECT_TRUE(names_eq(nn, {"S3"}));

   m_db.erase(s3);

   nn = get_names(u1->ModelSelectors());
   EXPECT_TRUE(names_eq(nn, {}));

   u1->includeModelSelector(*s1);
   u1->includeModelSelector(*s2);
   u1->includeModelSelector(*s3);

   m_db.erase(s3);

   nn = get_names(u1->ModelSelectors());
   EXPECT_TRUE(names_eq(nn, {"S2", "S1"}));

   m_db.erase(s1);

   nn = get_names(u1->ModelSelectors());
   EXPECT_TRUE(names_eq(nn, {"S2"}));

   m_db.erase(s2);
   EXPECT_TRUE(!s2->uid());

   nn = get_names(u1->ModelSelectors());
   EXPECT_TRUE(names_eq(nn, {}));

   // second deletion
   m_db.erase(s2);
   EXPECT_TRUE(!s2->uid());

   nn = get_names(u1->ModelSelectors());
   EXPECT_TRUE(names_eq(nn, {}));
}
