/*
 * Copyright (c) 2017 Rafal Lalik rafallalik@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "xmlparser.h"
#include <QString>
#include <cstdio>

using namespace std;

char* clone_string(const char* string) {
    int len;
    len = strlen(string);
    char* new_string = (char*) malloc(sizeof(char)*(len+1));
    if (new_string)
        strcpy(new_string, string);
    return new_string; 
}

XmlParser::XmlParser()
{
}

XmlParser::~XmlParser()
{
}

void XmlParser::clear()
{
}

void XmlParser::setDocument(const char * xml)
{
    clear();
    doc = QString::fromUtf8(xml);

    dom.setContent(doc);
    QDomElement title = dom.documentElement();
    
    items = title.elementsByTagName("item");
}

int XmlParser::parse()
{
    node_vec.clear();

    for (int i = 0; i < items.count(); ++i)
    {
        QDomNode node = items.item( i );
        QDomNamedNodeMap nodeAttributes = node.attributes();
        if ( nodeAttributes.namedItem( "type" ).nodeValue() == "QGraphicsTextItem" )
        {
            QDomNode lnode = node.namedItem( "content" ).firstChild();
            node_vec.push_back(lnode);
        }
    }

    return 1;
}

char * XmlParser::getNodeContent(uint i) const
{
    if (i >= node_vec.size())
        return nullptr;

    return clone_string(node_vec[i].nodeValue().toStdString().c_str());
}

void XmlParser::setNodeContent(uint i, const char * content)
{
    if (i >= node_vec.size())
        return;
    
    node_vec[i].setNodeValue(content);
}

char * XmlParser::getDocument() const
{
    return clone_string(dom.toString().toStdString().c_str());
}

void XmlParser::printDoc() const {
    printf("DOM = %s\n", dom.toString().toStdString().c_str());
}
