////
// @file Xml.cc
// @brief
// xml的解析库的封装，主要用于读取配置文件
//
// @author zjc
// @email zjc@email.com
//
#include "ndsl/utils/Xml.h"
#include "ndsl/utils/Error.h"
#include "ndsl/utils/Log.h"
#include <libxml/xpathInternals.h>
using namespace std;

namespace ndsl {
namespace utils {
Xml::Xml()
    : m_pdoc(NULL)
    , m_proot(NULL)
    , m_pbuf(NULL)
{}

Xml::~Xml()
{
    if (m_pdoc != NULL) { xmlFreeDoc(m_pdoc); }
    if (m_pbuf != NULL) { xmlBufferFree(m_pbuf); }
  
}

xmlNode *Xml::loadmemory(const char *buf, unsigned int size)
{
    // free doc handle
    if (m_pdoc != NULL) { xmlFreeDoc(m_pdoc); }

    // m_xmlType = XMLMEM;
    m_pdoc = xmlReadMemory(buf, size, NULL, "UTF-8", int(XML_PARSE_RECOVER));
    if (NULL == m_pdoc) {
        LOG(LOG_INFO_LEVEL, LOG_SOURCE_XML, "xml read memory error");
        return NULL;
    }
    xmlNodePtr node = xmlDocGetRootElement(m_pdoc);
    if (NULL == node) {
        LOG(LOG_INFO_LEVEL, LOG_SOURCE_XML, "empty file!");
        return NULL;
    }
    m_proot = node;
    return m_proot;
}

xmlNode *Xml::createxmlbuffer(const char *rootname)
{
    m_pdoc = xmlNewDoc(BAD_CAST "1.0");
    m_proot = xmlNewNode(NULL, BAD_CAST rootname);
    /* lint -e(534) */
    xmlDocSetRootElement(m_pdoc, m_proot); // return the old root element if any
                                           // was found, NULL if root was NULL

    return m_proot;
}
int Xml::getfirstnode(const char *xpath, xmlNode *&node)
{
   // LOG(LOG_INFO_LEVEL, LOG_SOURCE_XML, "madesdfhaoishdfoihasoidfhio");
    xmlXPathContextPtr context = xmlXPathNewContext(m_pdoc);
    if (NULL == context) {
        LOG(LOG_INFO_LEVEL, LOG_SOURCE_XML, "content is NULL");
        return S_FALSE;
    }
    xmlXPathObjectPtr result = xmlXPathEvalExpression(BAD_CAST xpath, context);
    xmlXPathFreeContext(context);
    if (NULL == result) {
        LOG(LOG_INFO_LEVEL,
            LOG_SOURCE_XML,
            "xmlXPathEvalExpression return NULL");
        xmlXPathFreeObject(result);
        return S_FALSE;
    }

    xmlNodeSetPtr pNodeset = result->nodesetval;
    if (pNodeset->nodeNr >= 1) {
        node = pNodeset->nodeTab[0];
    } else {
        LOG(LOG_INFO_LEVEL, LOG_SOURCE_XML, "no such path: %s", xpath);
        return S_FALSE;
    }
    xmlXPathFreeObject(result); // new add
    return S_OK;
    
}

int Xml::getnodeset(const char *xpath, std::vector<xmlNode *> &nodes)
{
    xmlXPathContextPtr context = xmlXPathNewContext(m_pdoc);
    if (NULL == context) {
        LOG(LOG_INFO_LEVEL, LOG_SOURCE_XML, "content is NULL");
        return S_FALSE;
    }
    xmlXPathObjectPtr result = xmlXPathEvalExpression(BAD_CAST xpath, context);
    xmlXPathFreeContext(context);
    if (NULL == result) {
        LOG(LOG_INFO_LEVEL,
            LOG_SOURCE_XML,
            "xmlXPathEvalExpression return NULL");
        xmlXPathFreeObject(result);
        return S_FALSE;
    }

    xmlNodeSetPtr pNodeset = result->nodesetval;
    for (int i = 0; i != pNodeset->nodeNr; ++i) {
        /*  lint -e(534) */
        nodes.push_back(pNodeset->nodeTab[i]); // no return value
    }
    xmlXPathFreeObject(result); // new add

    return S_OK;
}

int Xml::getchildren(const xmlNode *curnode, std::vector<xmlNode *> &children)
    const
{
    if (NULL == curnode) { return S_FALSE; }
    xmlNodePtr child = curnode->xmlChildrenNode;
    while (child != NULL) {
        if (xmlStrcmp(child->name, BAD_CAST "text")) //如果不是text
        {
            children.push_back(child);
        }
        child = child->next;
    }

    return S_OK;
}
int Xml::getspecialchildren(
    const xmlNode *curnode,
    const char *nodename,
    std::vector<xmlNode *> &nodes) const
{
    vector<xmlNode *> children;
    if (getchildren(curnode, children) != S_OK) {
        LOG(LOG_INFO_LEVEL, LOG_SOURCE_XML, "getChildren error");
        return S_FALSE;
    }

    vector<xmlNode *>::iterator vit;
    for (vit = children.begin(); vit != children.end(); ++vit) {
        if (!xmlStrcmp((*vit)->name, BAD_CAST nodename)) {
            /*  lint -e(534) */
            nodes.push_back(*vit); // no return value
        }
    }

    return S_OK;
}

int Xml::getproperty(xmlNode *node, const char *property, std::string &value)
    const
{
    xmlChar *attr = NULL;
    if (xmlHasProp(node, BAD_CAST property)) {
        attr = xmlGetProp(node, BAD_CAST property);
        value = (const char *) attr;
        xmlFree(attr);
    } else {
        LOG(LOG_INFO_LEVEL, LOG_SOURCE_XML, "no such property:  %s", property);
        return S_FALSE;
    }

    return S_OK;
    // need to xmlFree()
}

int Xml::setproperty(xmlNode *node, const char *property, const char *value)
{
    if (xmlHasProp(node, BAD_CAST property)) {
        if (NULL == xmlSetProp(node, BAD_CAST property, BAD_CAST value)) {
            LOG(LOG_INFO_LEVEL, LOG_SOURCE_XML, "xmlSetProp error");
        }
    } else {
        if (NULL == xmlNewProp(node, BAD_CAST property, BAD_CAST value)) {
            LOG(LOG_INFO_LEVEL, LOG_SOURCE_XML, "xmlNewProp error");
        }
    }

    return S_OK;
}

int Xml::delproperty(xmlNode *node, const char *property)
{
    xmlAttrPtr attr = NULL;
    if ((attr = xmlHasProp(node, BAD_CAST property)) != NULL) {
        return xmlRemoveProp(attr); // 0 if success and -1 in case of error
    } else {
        LOG(LOG_INFO_LEVEL, LOG_SOURCE_XML, "no such property:  %s", property);
        return S_FALSE;
    }
}

xmlNode *
Xml::addsimplechild(xmlNode *parent, const char *nodename, const char *value)
{
    return xmlNewTextChild(parent, NULL, BAD_CAST nodename, BAD_CAST value);
}

xmlNode *Xml::addchild(xmlNode *parent, xmlNode *cur)
{
    xmlNodePtr node = xmlCopyNode(
        cur,
        1); // if 1 do a recursive copy , if 2 copy properties and namespaces
    if (node != NULL) {
        return xmlAddChild(
            parent, node); // return the child or NULL in case of error
    } else {
        LOG(LOG_INFO_LEVEL, LOG_SOURCE_XML, "copy node error");
        return NULL;
    }
}


int Xml::getvalue(xmlNode *node, std::string &value)
{
    xmlChar *pValue = xmlNodeGetContent(node);
    if (pValue != NULL) {
        value = (const char *) pValue;
        xmlFree(pValue);
        return S_OK;
    } else {
        return S_FALSE;
    }
} 


void Xml::savefile()
{
    xmlSaveFile("test.xml",m_pdoc);
}


char* Xml::getxmlbuffer()
{
     return getnodebuffer(m_proot);
}


char* Xml::getnodebuffer(xmlNode *node)
{
    if ( m_pbuf != NULL )
    {
        xmlBufferFree( m_pbuf );
        m_pbuf = NULL;
    }

    m_pbuf = xmlBufferCreate();
    if ( m_pbuf == NULL )
    {
        LOG( LOG_INFO_LEVEL, LOG_SOURCE_XML,"xmlBufferCreate error" );
        return NULL;
    }

    int bytes = xmlNodeDump( m_pbuf, m_pdoc, node, 0, 1 );
    if ( bytes < 0 )
    {
        LOG( LOG_INFO_LEVEL, LOG_SOURCE_XML,"xmlNodeDump failed" );
        return NULL;
    }

    return ( char* )m_pbuf->content;
}
xmlNode *Xml::getrootnode() { return m_proot; }

} // namespace utils
} // namespace ndsl
