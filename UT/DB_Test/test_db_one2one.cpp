
#include "pch.h"
#include "test_db.h"

class One2One : public DB_Test
{
public:
};

NAMESPACE_TEST_F(DB_Relationships, One2One, IncludeExclude)
{
   u1->includePowerPin(*p1);
   EXPECT_TRUE(u1->countPowerPins() == 1);
   EXPECT_TRUE(p1->countPPinComps() == 1);
   EXPECT_TRUE(p1->parentPPinComp() == u1);
   EXPECT_TRUE(p2->countPPinComps() == 0);
   EXPECT_TRUE(p2->parentPPinComp() == nullptr);

   u1->includePowerPin(*p2);
   EXPECT_TRUE(u1->countPowerPins() == 1);
   EXPECT_TRUE(p1->countPPinComps() == 0);
   EXPECT_TRUE(p1->parentPPinComp() == nullptr);
   EXPECT_TRUE(p2->countPPinComps() == 1);
   EXPECT_TRUE(p2->parentPPinComp() == u1);

   u1->removePowerPin();
   EXPECT_TRUE(u1->countPowerPins() == 0);
   EXPECT_TRUE(p1->countPPinComps() == 0);
   EXPECT_TRUE(p1->parentPPinComp() == nullptr);
   EXPECT_TRUE(p2->countPPinComps() == 0);
   EXPECT_TRUE(p2->parentPPinComp() == nullptr);

   u1->includePowerPin(*p1);
   EXPECT_TRUE(u1->countPowerPins() == 1);
   EXPECT_TRUE(p1->countPPinComps() == 1);
   EXPECT_TRUE(p1->parentPPinComp() == u1);
   EXPECT_TRUE(p2->countPPinComps() == 0);
   EXPECT_TRUE(p2->parentPPinComp() == nullptr);
}

NAMESPACE_TEST_F(DB_Relationships, One2One, Deletion)
{
   u1->includePowerPin(*p1);
   EXPECT_TRUE(u1->countPowerPins() == 1);
   EXPECT_TRUE(p1->countPPinComps() == 1);
   EXPECT_TRUE(p1->parentPPinComp() == u1);
   EXPECT_TRUE(p2->countPPinComps() == 0);
   EXPECT_TRUE(p2->parentPPinComp() == nullptr);

   m_db.erase(p1);

   EXPECT_TRUE(u1->countPowerPins() == 0);
   EXPECT_TRUE(p2->countPPinComps() == 0);
   EXPECT_TRUE(p2->parentPPinComp() == nullptr);
}
