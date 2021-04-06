
#include "pch.h"
#include "test_db.h"

class ObjectsGeneral : public DB_Test
{
public:
};

NAMESPACE_TEST_F(DB_Objects, ObjectsGeneral, Create)
{
   auto* u2 = m_db.createComp();
   EXPECT_TRUE(u2);
   EXPECT_TRUE(u2->type() == eObjId::Comp);
   EXPECT_TRUE(u2->getName() == "");
   u2->setName("U2");

   names nn = get_names(m_db.Comps());
   EXPECT_TRUE(names_eq(nn, {"U1", "U2"}));
}
