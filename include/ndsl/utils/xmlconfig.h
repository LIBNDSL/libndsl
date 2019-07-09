

// XMLConfig.h
//
////
// @file XMLConfig.cpp
// @brief
//  xml文件解析类，用于配置文件的读取
//
// @author zjc
// @email zjc.cn@gmail.com
//
#ifndef UTIL_XMLCONFIG
#define UTIL_XMLCONFIG
#include<string>
#include<vector>

#include"pugixml.hpp"
#include "pugiconfig.hpp"

#define S_FLASE -1;
#define S_OK 0;
using namespace pugi;

class XMLConfig
{
    public:
        XMLConfig();
        XMLConfig(const char *filename);
       // XMLConfig(const char *filename);
        ~XMLConfig();
        //导入文件
        int LoadFile(const char* filename);
        //创建文件
        //XMLConfig *CreatFile(const char* name,const char* root);
        //保存为xml文件
        int Savefile(const char* filename);
        //获得满足xpath条件的第一个节点
        int GetFirstNode(const char* xpath,xml_node  &node);
        //获取xpath查询的所有节点
        int GetNodeSets(const char* xpath,xpath_node_set &sets);
        //int GetChildren(const pugi::)
        //获取,设置，删除节点的属性信息
        int GetProperty(xml_node &node,const char* property,std:: string &value);

        //int SetProperty(xml_node &node,const char* property,const char *value);
        int DelProperty(xml_node &node,const char* property);
        //设置节点属性和值
        int SetNodeTextAndAttr(xml_node &node,const char *text,const char *attr,const char *value );
        //删除子节点
        int DelChidNode(xml_node node,const char *child);
        //插入子节点
        int AddChildNode(xml_node &parentnode, const char* nodename);
        //将节点插入xml_doc
        int AddNode(xml_node &node);

    private:
    std::string m_filename;
    xml_document m_doc;
    xml_node m_node;
};
#endif