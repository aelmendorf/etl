/******************************************************************************
The MIT License(MIT)

Embedded Template Library.
https://github.com/ETLCPP/etl
http://www.etlcpp.com

Copyright(c) 2017 jwellbelove

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

#include "UnitTest++.h"
#include "ExtraCheckMacros.h"

#include "factory.h"

#include <string>
#include <type_traits>

namespace
{
  bool destructor;

  //***********************************
  struct Base
  {
    Base()
    {
      destructor = false;
    }

    virtual ~Base()
    {
    }

    virtual void Set() = 0;
  };

  //***********************************
  struct Derived1 : public Base
  {
    int i;

    Derived1()
      : i(0)
    {
    }

    ~Derived1()
    {
      destructor = true;
    }

    void Set()
    {
      i = 1;
    }
  };

  //***********************************
  struct Derived2 : public Base
  {
    double d;

    Derived2()
      : d(0.0)
    {
    }

    ~Derived2()
    {
      destructor = true;
    }

    void Set()
    {
      d = 1.2;
    }
  };

  //***********************************
  struct Derived3 : public Base
  {
    std::string s;

    Derived3()
      : s("constructed")
    {
    }

    Derived3(const char* p1)
      : s("constructed")
    {
      s.append(p1);
    }

    Derived3(const char* p1, const std::string& p2)
      : s("constructed")
    {
      s.append(p1);
      s.append(p2);
    }

    Derived3(const char* p1, const std::string& p2, const char* p3)
      : s("constructed")
    {
      s.append(p1);
      s.append(p2);
      s.append(p3);
    }

    Derived3(const char* p1, const std::string& p2, const char* p3, const std::string& p4)
      : s("constructed")
    {
      s.append(p1);
      s.append(p2);
      s.append(p3);
      s.append(p4);
    }

    ~Derived3()
    {
      destructor = true;
    }

    void Set()
    {
      s = "set";
    }
  };

  enum
  {
    DERIVED1,
    DERIVED2,
    DERIVED3
  };

  typedef etl::type_id_pair<Derived1, DERIVED1> D1_Type;
  typedef etl::type_id_pair<Derived2, DERIVED2> D2_Type;
  typedef etl::type_id_pair<Derived3, DERIVED3> D3_Type;

  typedef etl::factory<4, Base, D1_Type, D2_Type, D3_Type> Factory;

  SUITE(test_factory)
  {
    //*************************************************************************
    TEST(test_sizes)
    {
      Factory factory;

      size_t ms = Factory::MAX_SIZE;
      CHECK_EQUAL(4U, ms);
      CHECK_EQUAL(4U, factory.max_size());
      CHECK_EQUAL(4U, factory.available());
      CHECK_EQUAL(0U, factory.size());
      CHECK(factory.empty());
      CHECK(!factory.full());

      factory.create_from_type<Derived1>();
      CHECK_EQUAL(3U, factory.available());
      CHECK_EQUAL(1U, factory.size());
      CHECK(!factory.empty());
      CHECK(!factory.full());

      factory.create_from_type<Derived1>();
      factory.create_from_type<Derived1>();
      factory.create_from_type<Derived1>();
      CHECK_EQUAL(0U, factory.available());
      CHECK_EQUAL(4U, factory.size());
      CHECK(!factory.empty());
      CHECK(factory.full());

      CHECK_THROW(factory.create_from_type<Derived1>(), etl::factory_cannot_create);
    }

    //*************************************************************************
    TEST(test_create_release)
    {
      Factory factory;

      Base* p;

      p = factory.create_from_type<Derived1>();
      Derived1* pd1 = static_cast<Derived1*>(p);
      CHECK_EQUAL(0, pd1->i);
      p->Set();
      CHECK_EQUAL(1, pd1->i);
      factory.destroy(p);
      CHECK(destructor);

      destructor = false;
      p = factory.create_from_type<Derived2>();
      Derived2* pd2 = static_cast<Derived2*>(p);
      CHECK_EQUAL(0.0, pd2->d);
      p->Set();
      CHECK_EQUAL(1.2, pd2->d);
      factory.destroy(p);
      CHECK(destructor);

      destructor = false;
      p = factory.create_from_type<Derived3>();
      Derived3* pd3 = static_cast<Derived3*>(p);
      CHECK_EQUAL("constructed", pd3->s);
      p->Set();
      CHECK_EQUAL("set", pd3->s);
      factory.destroy(p);
      CHECK(destructor);
    }

    //*************************************************************************
    TEST(test_create_release_const)
    {
      Factory factory;

      const Derived1& d = *factory.create_from_type<Derived1>();

      CHECK_EQUAL(0, d.i);
      factory.destroy(&d);
      CHECK(destructor);

    }

    //*************************************************************************
    TEST(test_create_emplace)
    {
      Factory factory;

      Base* p;
      Derived3* pd3;

      p = factory.create_from_type<Derived3>("1");
      pd3 = static_cast<Derived3*>(p);
      CHECK_EQUAL("constructed1", pd3->s);
      factory.destroy(p);

      p = factory.create_from_type<Derived3>("1", "2");
      pd3 = static_cast<Derived3*>(p);
      CHECK_EQUAL("constructed12", pd3->s);
      factory.destroy(p);

      p = factory.create_from_type<Derived3>("1", "2", "3");
      pd3 = static_cast<Derived3*>(p);
      CHECK_EQUAL("constructed123", pd3->s);
      factory.destroy(p);

      p = factory.create_from_type<Derived3>("1", "2", "3", "4");
      pd3 = static_cast<Derived3*>(p);
      CHECK_EQUAL("constructed1234", pd3->s);
      factory.destroy(p);
    }

    //*************************************************************************
    TEST(test_did_not_create)
    {
      Factory factory1;
      Factory factory2;

      Base* p;

      p = factory1.create_from_type<Derived1>();
      CHECK_NO_THROW(factory1.destroy(p));

      p = factory2.create_from_type<Derived1>();
      CHECK_THROW(factory1.destroy(p), etl::factory_did_not_create);
    }

    //*************************************************************************
    TEST(test_create_from_index)
    {
      Factory factory;
      Base* p;
      Derived3* p3;

      CHECK_NO_THROW(p = factory.create_from_id<DERIVED1>());
      CHECK(dynamic_cast<Derived1*>(p) != 0);
      factory.destroy(p);
      
      CHECK_NO_THROW(p = factory.create_from_id<DERIVED2>());
      CHECK(dynamic_cast<Derived2*>(p) != 0);
      factory.destroy(p);

      CHECK_NO_THROW(p = factory.create_from_id<DERIVED3>());
      CHECK(dynamic_cast<Derived3*>(p) != 0);
      factory.destroy(p);

      CHECK_NO_THROW(p = factory.create_from_id<DERIVED3>("1"));
      p3 = static_cast<Derived3*>(p);
      CHECK_EQUAL("constructed1", p3->s);
      factory.destroy(p);
    }
  };
}
