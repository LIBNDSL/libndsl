#include "xmlconfig.h"
#include <iostream>

XMLConfig::XMLConfig():
    m_doc(),
    m_node(NULL)
{
}

XMLConfig::XMLConfig(const char *filename):
    m_doc(),m_node(NULL)
    {
        if(!m_doc.load_file(filename))
        {
            std::cout<<"loadFile error";
        }
    }
XMLConfig::~XMLConfig()
{}

int XMLConfig::LoadFile(const char* filename){
   m_filename=filename;
   if(m_doc.load_file(filename))
    {
        return S_OK;
    }
    else
    return S_FLASE;

   //m_doc=pugi::xml
}

int XMLConfig::SetNodeTextAndAttr(xml_node &node,const char *text,const char *attr,const char *value)
{
    node.text().set(text);
    node.append_attribute(attr).set_value(value);
    return S_OK;
}

int XMLConfig::GetProperty(xml_node &node,const char* property,std::string &value)
{
    value=node.child(property).text().get();
    return S_OK;

}

int XMLConfig::DelProperty(xml_node &node,const char *property)
{
    node.remove_attribute(property);
    return S_OK;

}
int XMLConfig::DelChidNode(xml_node node,const char *child)
{
    node.remove_child(child);
    return S_OK;
}

int XMLConfig::Savefile(const char *filename)
{
    m_doc.save_file(filename);
    return S_OK;

}

 int XMLConfig::GetNodeSets(const char* xpath,xpath_node_set &sets)
 {
     xpath_query query_remote_tool(xpath);
     sets=query_remote_tool.evaluate_node_set(m_doc);
     return S_OK;

 }

int XMLConfig::GetFirstNode(const char* xpath,xml_node &node)
{
    node=m_doc.first_element_by_path(xpath);
    return S_OK;
    

}

int XMLConfig::AddChildNode(xml_node &parentnode ,const char *nodename){

    if(parentnode.append_child(nodename))
    {
        return S_OK;
    }
    else 
    return S_FLASE;

}

/*int XMLConfig::AddNode(xml_node &node){
    m_doc.append_child(node);

}
*/

