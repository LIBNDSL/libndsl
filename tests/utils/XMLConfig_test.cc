#define CATCH_CONFIG_MAIN
#include"catch.hpp"
#include "xmlconfig.h"
TEST_CASE("XMLConfigtest"){
    XMLConfig xmlconfig("phone.xml");
    xml_node node(NULL);
    xpath_node_set set;
    std::string str;
    SECTION("LoadFile")
    {
       REQUIRE(xmlconfig.LoadFile("phone.xml")==0);
    }
    SECTION("GetFirstNode")
    {
        REQUIRE(xmlconfig.GetFirstNode("Phone_books",node)==0);
    }
    SECTION("GetNodeSets")
    {
        REQUIRE(xmlconfig.GetNodeSets("Phone_books",set)==0);
    }
    SECTION("ModifyProperty")
    {
        xmlconfig.GetFirstNode("Phone_books/phone",node);
       
        REQUIRE(xmlconfig.GetProperty(node,"id",str)==0);
        REQUIRE(xmlconfig.DelProperty(node,"id")==0);
        REQUIRE(xmlconfig.DelChidNode(node,"tel")==0);
        REQUIRE(xmlconfig.AddChildNode(node,"shit")==0);
        REQUIRE(xmlconfig.SetNodeTextAndAttr( node,"friend","hap","ship")==0);
        

        //REQUIRE()
       // REQUIRE()
       // REQUIRE(xmlconfig.GetFirstNode("Phone_books/phone",node)==0);
    }
}