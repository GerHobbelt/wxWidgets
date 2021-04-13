#pragma once

#include "test_db/generated/database.h"
#include "test_db/generated/database_traits.h"
#include "test_db/generated/database_traits_types.h"

#include "filesystem"

class DB_Test : public ::testing::Test
{
public:
   shm::shared_memory *m_shared_mem;
   cDatabase *m_db;
   cRelationship *m_r;
   cComp *u1, *u2, *u3;
   cPin *p1, *p2, *p3;
   cModelSelector *s1, *s2, *s3;

   void SetUp() override
   {
      namespace fss = std::filesystem;
      const char* shared_seg_name = "test_data";
      if (fss::exists(shared_seg_name)) {
         fss::remove(shared_seg_name);
      }
      m_shared_mem = shm::create(shared_seg_name);
      m_db = m_shared_mem->construct<cDatabase>("db")();
      m_r = m_db->create<cRelationship>();
      u1 = create_comp("U1");
      u2 = u3 = nullptr;
      p1 = create_pin("U1.1");
      p2 = create_pin("U1.2");
      p3 = create_pin("U1.3");
      s1 = create_model_selector("S1");
      s2 = create_model_selector("S2");
      s3 = create_model_selector("S3");
   }
   void TearDown() override
   {
      m_db->erase(s3);
      m_db->erase(s2);
      m_db->erase(s1);
      m_db->erase(p3);
      m_db->erase(p2);
      m_db->erase(p1);
      m_db->erase(u1);
      m_db->erase(u2);
      m_db->erase(u3);
      m_shared_mem->destroy<cDatabase>("db");
      shm::destroy(m_shared_mem);
   }

   using free_list = std::set<size_t>;

   cComp* create_comp(const char* name)
   {
      auto p = m_db->createComp();
      EXPECT_TRUE(p);
      EXPECT_TRUE(p->countPins() == 0);
      p->setName(name);
      return p;
   }
   cPin* create_pin(const char* name)
   {
      auto p = m_db->createPin();
      EXPECT_TRUE(p);
      EXPECT_TRUE(p->countParentComps() == 0);
      EXPECT_TRUE(p->parentComp() == nullptr);
      p->setName(name);
      return p;
   }
   cModelSelector* create_model_selector(const char* name)
   {
      auto p = m_db->createModelSelector();
      EXPECT_TRUE(p);
      EXPECT_TRUE(p->countComps() == 0);
      p->setName(name);
      return p;
   }

   auto get_free_list()
   {
      free_list free_idx;
      for (auto p = m_r->first_free(); p; p = m_r->next_free(p)) {
         free_idx.insert(p - m_r->m_data);
      }
      return free_idx;
   }
   bool free_list_eq(const free_list& f1)
   {
      const free_list& f2 = get_free_list();
      auto s1 = f1.size(), s2 = f2.size();
      if (s1 != s2) {
         return false;
      }
      for (auto e: f1) {
         if (!m_r->is_free(m_r->m_data[e])) {
            return false;
         }
         if (f2.find(e) == f2.end()) {
            return false;
         }
      }
      return true;
   }

   using names = std::vector<std::string>;
   bool names_eq(const names& n1, const names& n2)
   {
      if (n1.size() != n2.size()) {
         return false;
      }
      auto s2 = n2.begin();
      for (auto s: n1) {
         if (s != *s2++) {
            return false;
         }
      }
      return true;
   }

   template <class T>
   names get_names(T&& range)
   {
      names nn;
      for (auto& i: range) {
         nn.push_back(i.getName());
      }
      return nn;
   }
};
