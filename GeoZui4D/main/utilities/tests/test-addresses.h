// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

// Give the address interface a workout.  Tell emacs  -*- c++ -*-

#include <cxxtest/TestSuite.h>

#include <iostream>
#include <sstream>
//#include <memory> // auto_ptr

#include "Addressable.h"
#include "DottedAddress.h"

/// \file
/// \brief Test the Addressable and DottedAddress classes

//////////////////////////////////////////////////////////////////////

bool strequ(const char *s1, const char *s2) {
  if (0==strcmp(s1,s2)) return true;
  return false;
}

/// \brief Test Suite using cxxtest
class AddressTestSuite : public CxxTest::TestSuite 
{
 public:

  /// Work with the Addressable to make sure an empty address works right.
  void testAddressableEmpty(void) {
    cout << "  Addressable Empty\n";
    Addressable a;
    stringstream s;
    a.getHtmlInfo(s,1);
    //cout << endl
    //     << "  Should be homeless" << endl
    //	 << "    Addressable getHtmlInfo: " << s.str()<<endl;;
    TS_ASSERT_EQUALS((void*)0,a.getAddress());
    a.publishName("foo");
    TS_ASSERT((void*)0!=a.getAddress())
    return;
  }

  /// It looks like Addressable is not the way to use the tree.
  void testDottedAddressSimple(void) {
    cout << "  testDottedAddressSimple\n";
    DottedAddress *da1 = new DottedAddress(0);
    TS_ASSERT_EQUALS((void*)0,da1->getName());
    TS_ASSERT_EQUALS((void*)0,da1->getAddress());
    return;
  }

  /// Test setting the name
  void testDottedAddressName(void) {
    cout << "  \n";
    DottedAddress *root = new DottedAddress(0,"NAME");
    //TS_ASSERT(0,strcmp(root->getName(),"NAME"));
    TS_ASSERT(strequ(root->getName(),"NAME"));
    return;
  }

  /// Play with just two dotted address nodes
  void testDottedAddress2level(void) {
    DottedAddress *root = new DottedAddress(0,"name_top");
    DottedAddress *bottom = new DottedAddress(0,"name_bot");
    root->addChild(bottom);
    TS_ASSERT_EQUALS(root->getNumChildren(),1);
    TS_ASSERT_EQUALS(bottom->getNumChildren(),0);
    TS_ASSERT_EQUALS(root->getParent(),(void*)0);
    TS_ASSERT_EQUALS(bottom->getParent(),root);
    TS_ASSERT_EQUALS(root->getChild(),bottom);
    TS_ASSERT_EQUALS(root->getChild()->getParent(),root); // loop test
    cout << "  Root address:   " << root->getAddress() << endl;
    cout << "  Bottom address: " << bottom->getAddress() << endl;
    TS_ASSERT(strequ(root->getName(),"name_top"));
    TS_ASSERT_EQUALS(root->find("name_bot"),bottom);
    //TS_ASSERT_EQUALS(root->find("name_top.name_bot"),bottom);  // NOT TRUE?!?!

    //cout << "  Trying removeChild" << endl;
    root->removeChild(bottom); 
    TS_ASSERT_EQUALS(root->getNumChildren(),0);
    // THIS is not true...!
    //TS_ASSERT_EQUALS(bottom->getParent(),(void*)0);
  }	


  /// Build a tree and draw it with graphviz
  void testDottedAddressGraphViz(void) {
    DottedAddress *root = new DottedAddress(0,"I_am_root");
    for (int i=0;i<5;i++) {
      char s[100];
      sprintf(s,"c%d",i);
      cout << "Adding "<< s<<endl;
      root->addChild(new DottedAddress(0,s));
      cout << "TREE " << i << endl;
    }

    for (int i=0;i<2;i++) {
      char s[100];
      sprintf(s,"cc%d",i);
      cout << "Adding "<< s<<endl;
      root->getChild(1)->addChild(new DottedAddress(0,s));
    }

    for (int i=0;i<3;i++) {
      char s[100];
      sprintf(s,"cd%d",i);
      cout << "Adding "<< s<<endl;
      root->getChild(4)->addChild(new DottedAddress(0,s));
    }

    cout << "TREE: " << root->getName() << endl;
    root->printTextGraph(1);

    root->writeGraphViz("foo");
  }

  void testDottedAddressLonger(void) {
    DottedAddress *root = new DottedAddress(0,"Owned");
    cout << "root address: " << root->getAddress() << endl;
    TS_ASSERT(strequ(root->getAddress(),"Owned"));
    DottedAddress *l1=new DottedAddress(0,"l1");
    root->addChild(l1);
    cout << "l1 address: " << l1->getAddress() << endl;
    TS_ASSERT(strequ(l1->getAddress(),"Owned.l1"));

    DottedAddress *l2=new DottedAddress(0,"l2");
    l1->addChild(l2);
    cout << "l2 address: " << l2->getAddress() << endl;
    TS_ASSERT(strequ(l2->getAddress(),"Owned.l1.l2"));
    
    DottedAddress *l3=new DottedAddress(0,"l3");
    DottedAddress *l4=new DottedAddress(0,"l4");
    DottedAddress *l5=new DottedAddress(0,"l5");
    l2->addChild(l3);
    l3->addChild(l4);
    l4->addChild(l5);
    cout << "l5 address: " << l5->getAddress() << endl;
    TS_ASSERT(strequ(l5->getAddress(),"Owned.l1.l2.l3.l4.l5"));
    
    return;
  }

  void testDottenAddressRemoveChild(void) {
    DottedAddress *root = new DottedAddress(0,"Owned");
    cout << "root address: " << root->getAddress() << endl;
    DottedAddress *l1=new DottedAddress(0,"l1");
    root->addChild(l1);
    root->removeChild(l1);
    TS_ASSERT_EQUALS(root->getNumChildren(),0);
  }

  void testDottenAddressDeleteWithChild(void) {
    DottedAddress *root = new DottedAddress(0,"Owned");
    cout << "root address: " << root->getAddress() << endl;
    DottedAddress *l1=new DottedAddress(0,"l1");
    root->addChild(l1);
    delete root;
  }

  void testDottenAddressDeleteWithManyChilden(void) {
    DottedAddress *root = new DottedAddress(0,"Owned");
    cout << "root address: " << root->getAddress() << endl;
    for (int i=0;i<100;i++) root->addChild(new DottedAddress(0,"somename"));
    delete root;
  }

  /// Watch the last child get stuffed into the middle
  void testDottedAddressDeleteFromManyChilden(void) {
    DottedAddress *root = new DottedAddress(0,"Owned");
    cout << "root address: " << root->getAddress() << endl;
    for (int i=0;i<2;i++) root->addChild(new DottedAddress(0,"somename1"));
    DottedAddress *l1=new DottedAddress(0,"l1");  root->addChild(l1);
    for (int i=0;i<2;i++) root->addChild(new DottedAddress(0,"somename2"));
    DottedAddress *last = new DottedAddress(0,"somename3  WATCH THIS ONE");
    root->addChild(last);
    cout << "before delete" << endl;
    root->printTextGraph(1);
    delete l1;
    cout << "after delete" << endl;
    root->printTextGraph(1);
    TS_ASSERT_EQUALS(root->getChild(2),last);
    delete root;
  }

  void testDAddrDelOne(void) {
    cout << "  DAddrDelOne\n";
    Addressable *payload=new Addressable(); // This prevents the root from getting freed.
    DottedAddress *r = new DottedAddress((Addressable*)payload,"r",1); // root
    DottedAddress *c = new DottedAddress(0,"c"); // child
    r->addChild(c);
    TS_ASSERT_EQUALS(r->getNumChildren(),1);
    r->removeChild(c);
    TS_ASSERT_EQUALS(r->getNumChildren(),0);
    // Is the world falling down?
    cerr << "foo1"<<endl;
    #if 1
    // FIX: HELP... this is causing a crash?!?!
    int *ints=new int[20];
    for (int i=0; i<20;i++) {
      ints[i]=i;
    }
    #endif
    cerr << "foo2"<<endl;
    TS_ASSERT_EQUALS(r->getNumChildren(),0);
    
  }


  void testAddressableEmpty2(void) {
    cout << "Starting test Empty2" <<endl;
    Addressable a;
    stringstream s;
    TS_ASSERT_EQUALS((void*)0,a.getAddress());
    a.publishName("foo");
    TS_ASSERT((void*)0!=a.getAddress())
    return;
  }

};
