
#include "pch.h"
#include "test_db.h"

class One2Many : public DB_Test
{
public:
};

NAMESPACE_TEST_F(DB_Relationships, One2Many, FreeList)
{
   auto fl = m_r.free_list_header();
   EXPECT_TRUE(fl == nullptr);

   auto f = m_r.pop_free();
   EXPECT_TRUE(f == nullptr);

   EXPECT_TRUE(free_list_eq({}));

   const int size = 10;
   m_r.resize(size);

   EXPECT_TRUE(m_r.size() == size);
   free_list tmp;
   for (int i = 0; i < size; ++i) {
      tmp.insert(i);
   }
   EXPECT_TRUE(free_list_eq(tmp));

   int count = 0;
   while (m_r.pop_free()) {
      // as we pop the last free item, the free header is also popped
      ++count;
   }
   EXPECT_TRUE(count == size);
   EXPECT_TRUE(free_list_eq({}));

   f = m_r.pop_free();
   EXPECT_TRUE(f == nullptr);

   int seq[] = {3, 5, 7};
   for (size_t i = 0; i < std::size(seq); ++i) {
      m_r.push_free(m_r.m_data + seq[i]);
   }
   EXPECT_TRUE(free_list_eq({7, 5, 3, size})); // the free header is recreated, so total size+1 elements now

   f = m_r.pop_free();
   EXPECT_TRUE(f == m_r.m_data + 7);
   EXPECT_TRUE(free_list_eq({5, 3, size}));

   f = m_r.pop_free();
   EXPECT_TRUE(f == m_r.m_data + 5);
   EXPECT_TRUE(free_list_eq({3, size}));

   f = m_r.pop_free();
   EXPECT_TRUE(f == m_r.m_data + 3);
   EXPECT_TRUE(free_list_eq({size}));

   int seq2[] = {0, 1, 2, 3};
   for (size_t i = 0; i < std::size(seq2); ++i) {
      m_r.push_free(m_r.m_data + seq2[i]);
   }
   EXPECT_TRUE(free_list_eq({3, 2, 1, 0, size}));

   auto p = m_r.first_free();
   EXPECT_TRUE(p == m_r.m_data + 3);
   p = m_r.next_free(p);
   EXPECT_TRUE(p == m_r.m_data + 2);
   p = m_r.next_free(p);
   EXPECT_TRUE(p == m_r.m_data + 1);
   p = m_r.next_free(p);
   EXPECT_TRUE(p == m_r.m_data + 0);
   p = m_r.next_free(p);
   EXPECT_TRUE(p == m_r.m_data + size);
   p = m_r.next_free(p);
   EXPECT_TRUE(p == nullptr);

   f = m_r.pop_free();
   EXPECT_TRUE(f == m_r.m_data + 3);
   f = m_r.pop_free();
   EXPECT_TRUE(f == m_r.m_data + 2);
   f = m_r.pop_free();
   EXPECT_TRUE(f == m_r.m_data + 1);
   f = m_r.pop_free();
   EXPECT_TRUE(f == m_r.m_data + 0);
   f = m_r.pop_free();
   EXPECT_TRUE(f == m_r.m_data + size);
   f = m_r.pop_free();
   EXPECT_TRUE(f == nullptr);
   f = m_r.pop_free();
   EXPECT_TRUE(f == nullptr);

   for (size_t i = 0; i < std::size(seq2); ++i) {
      m_r.push_free(m_r.m_data + seq2[i]);
   }
   EXPECT_TRUE(free_list_eq({3, 2, 1, 0, size+1}));

   m_r.resize(2 * size);

   tmp.clear();
   for (int i = 1; i < size; ++i) {
      tmp.insert(i + size);
   }
   EXPECT_TRUE(free_list_eq({tmp}));
}

NAMESPACE_TEST_F(DB_Relationships, One2Many, IncludeExclude)
{
   u1->includePin(*p1);
   EXPECT_TRUE(u1->countPins() == 1);
   EXPECT_TRUE(p1->countComps() == 1);
   EXPECT_TRUE(p1->parentComp() == u1);
   EXPECT_TRUE(p2->countComps() == 0);
   EXPECT_TRUE(p2->parentComp() == nullptr);

   u1->includePin(*p2);
   EXPECT_TRUE(u1->countPins() == 2);
   EXPECT_TRUE(p1->countComps() == 1);
   EXPECT_TRUE(p1->parentComp() == u1);
   EXPECT_TRUE(p2->countComps() == 1);
   EXPECT_TRUE(p2->parentComp() == u1);

   u1->excludePin(*p1);
   EXPECT_TRUE(u1->countPins() == 1);
   EXPECT_TRUE(p1->countComps() == 0);
   EXPECT_TRUE(p1->parentComp() == nullptr);
   EXPECT_TRUE(p2->countComps() == 1);
   EXPECT_TRUE(p2->parentComp() == u1);

   u1->includePin(*p1);
   EXPECT_TRUE(u1->countPins() == 2);
   EXPECT_TRUE(p1->countComps() == 1);
   EXPECT_TRUE(p1->parentComp() == u1);
   EXPECT_TRUE(p2->countComps() == 1);
   EXPECT_TRUE(p2->parentComp() == u1);

   u1->excludePin(*p2);
   EXPECT_TRUE(u1->countPins() == 1);
   EXPECT_TRUE(p1->countComps() == 1);
   EXPECT_TRUE(p1->parentComp() == u1);
   EXPECT_TRUE(p2->countComps() == 0);
   EXPECT_TRUE(p2->parentComp() == nullptr);

   u1->excludePin(*p1);
   EXPECT_TRUE(u1->countPins() == 0);
   EXPECT_TRUE(p1->countComps() == 0);
   EXPECT_TRUE(p1->parentComp() == nullptr);
   EXPECT_TRUE(p2->countComps() == 0);
   EXPECT_TRUE(p2->parentComp() == nullptr);

   u1->includePin(*p2);
   EXPECT_TRUE(u1->countPins() == 1);
   EXPECT_TRUE(p1->countComps() == 0);
   EXPECT_TRUE(p1->parentComp() == nullptr);
   EXPECT_TRUE(p2->countComps() == 1);
   EXPECT_TRUE(p2->parentComp() == u1);

   auto* u2 = m_db.createComp();
   EXPECT_TRUE(u2);
   EXPECT_TRUE(u2->type() == eObjId::Comp);
   EXPECT_TRUE(u2->getName() == "");
   u2->setName("U2");

   u2->includePin(*p2);
   EXPECT_TRUE(u1->countPins() == 0);
   EXPECT_TRUE(p1->countComps() == 0);
   EXPECT_TRUE(p1->parentComp() == nullptr);
   EXPECT_TRUE(p2->countComps() == 1);
   EXPECT_TRUE(p2->parentComp() == u2);
}

NAMESPACE_TEST_F(DB_Relationships, One2Many, Iteration)
{
   names nn = get_names(u1->Pins());
   EXPECT_TRUE(names_eq(nn, {}));

   u1->includePin(*p1);
   u1->includePin(*p2);

   nn = get_names(u1->Pins());
   EXPECT_TRUE(names_eq(nn, { "U1.1", "U1.2" }));

   nn.clear();
   auto&& r = u1->Pins();
   for (auto ipin = r.begin(); ipin != r.end(); ++ipin) {
      nn.push_back(ipin->m_Name);
   }
   EXPECT_TRUE(names_eq(nn, { "U1.1", "U1.2" }));

   nn.clear();
   for (auto& ipin : u1->Pins()) {
      nn.push_back(ipin.m_Name);
      if (ipin.m_Name == "U1.2") {
         u1->includePin(*p3);
      }
   }
   EXPECT_TRUE(names_eq(nn, { "U1.1", "U1.2", "U1.3" }));

   u1->excludePin(*p1);
   nn = get_names(u1->Pins());
   EXPECT_TRUE(names_eq(nn, {"U1.2", "U1.3"}));

   u1->excludePin(*p2);
   nn = get_names(u1->Pins());
   EXPECT_TRUE(names_eq(nn, {"U1.3"}));

   u1->excludePin(*p3);
   nn = get_names(u1->Pins());
   EXPECT_TRUE(names_eq(nn, {}));

   u1->includePin(*p1);
   u1->includePin(*p2);
   u1->includePin(*p3);

   nn = get_names(u1->Pins());
   EXPECT_TRUE(names_eq(nn, {"U1.3", "U1.2", "U1.1"}));

   u1->excludePin(*p3);
   nn = get_names(u1->Pins());
   EXPECT_TRUE(names_eq(nn, {"U1.2", "U1.1"}));
}

NAMESPACE_TEST_F(DB_Relationships, One2Many, Deletion)
{
   u1->includePin(*p1);
   u1->includePin(*p2);
   u1->includePin(*p3);

   names nn = get_names(u1->Pins());
   EXPECT_TRUE(names_eq(nn, {"U1.1", "U1.2", "U1.3" }));

   m_db.erase(p1);

   nn = get_names(u1->Pins());
   EXPECT_TRUE(names_eq(nn, {"U1.2", "U1.3"}));

   m_db.erase(p2);

   nn = get_names(u1->Pins());
   EXPECT_TRUE(names_eq(nn, {"U1.3"}));

   m_db.erase(p3);

   nn = get_names(u1->Pins());
   EXPECT_TRUE(names_eq(nn, {}));

   p1 = create_pin("U1.1");
   p2 = create_pin("U1.2");
   p3 = create_pin("U1.3");
   u1->includePin(*p1);
   u1->includePin(*p2);
   u1->includePin(*p3);

   m_db.erase(p3);

   nn = get_names(u1->Pins());
   EXPECT_TRUE(names_eq(nn, {"U1.2", "U1.1"}));

   m_db.erase(p1);

   nn = get_names(u1->Pins());
   EXPECT_TRUE(names_eq(nn, {"U1.2"}));

   m_db.erase(p2);
   EXPECT_TRUE(!p2->uid());

   nn = get_names(u1->Pins());
   EXPECT_TRUE(names_eq(nn, {}));

   // second deletion
   m_db.erase(p2);
   EXPECT_TRUE(!p2->uid());

   nn = get_names(u1->Pins());
   EXPECT_TRUE(names_eq(nn, {}));
}
