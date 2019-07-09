////
// @fiel XmlTest.cc
// @brief
// Xml 测试类
//
// @author zjc
// @email 2324234234@qq.com
//
#include<iostream>
#include "../catch.hpp"
#include "ndsl/utils/Xml.h"
#include "ndsl/utils/Error.h"
using namespace std;
TEST_CASE("Xml") {
    ndsl::utils::Xml xml;
    //  xmlNodePtr nodeptr = NULL;
    //xmlNode node;

    xmlNodePtr nodeptr,childnodeptr;// = &node;

    std::vector<xmlNode *> nodes;
    string str;
    // std::vector<xmlNode> *nodesptr = &nodes;

    char buf[]=R"(<?xml version="1.0" encoding="UTF-8"?>
<phone_books>
  <phone id="1">
     <name>Anker</name>
     <tel>18923873456</tel>
     <address>Shenzheng</address>
  </phone>
  <phone id="2">
    <name>Jermey</name>
    <tel>18623873456</tel>
    <address>Beijing</address>
  </phone>
  <phone id="3">
    <name>Lili</name>
    <tel>13223873456</tel>
    <address>Shanghai</address>
  </phone>
</phone_books>
  )";
    unsigned len=strlen(buf);
    
    xml.loadmemory(buf, len);
    
    
    
  
    REQUIRE(xml.getnodeset("phone_books/phone", nodes)==S_OK);

    xml.getfirstnode("/phone_books", nodeptr );
    REQUIRE(xml.getchildren(nodeptr,nodes)==S_OK);
    REQUIRE(xml.getspecialchildren(nodeptr,"phone",nodes)==S_OK);

    xml.getfirstnode("/phone_books/phone", nodeptr);
    REQUIRE(xml.getvalue(nodeptr,str)==S_OK);
   
    REQUIRE(xml.getproperty(nodeptr,"id",str)==S_OK);
  
    REQUIRE(xml.setproperty(nodeptr,"id","12")==S_OK);
    REQUIRE(xml.delproperty(nodeptr,"id")==S_OK);
    REQUIRE(xml.addsimplechild(nodeptr,"type","home")!=NULL);
    xml.getfirstnode("/phone_books/phone/tel", childnodeptr);
    REQUIRE(xml.addchild(nodeptr,childnodeptr)!=NULL);
    
    REQUIRE(xml.getrootnode()!=NULL);
    REQUIRE(xml.getxmlbuffer()!=NULL);
    xml.savefile();

}