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

#ifndef XMLPARSER_H
#define XMLPARSER_H

#ifdef __cplusplus

#include <string>
#include <vector>

#include <QDomElement>

using namespace std;


class XmlParser
{
public:
    XmlParser();
    virtual ~XmlParser();

    void setDocument(const char * xml);

    int parse();
    uint getContentNodesNumber() const { return node_vec.size(); }

    char * getNodeContent(uint i) const;
    void setNodeContent(uint i, const char *);

    void printDoc() const;

    void clear();

    char * getDocument() const;

//     void debug() const { for (Frame f : frames) f.print(); }
private:

private:
    QString doc;
    QDomDocument dom;
    QDomNodeList items;
    std::vector<QDomNode> node_vec;
};

#else
typedef
    void *
        XmlParser;
#endif

#ifdef __cplusplus
extern "C" {
#endif
#if defined(__STDC__) || defined(__cplusplus)

extern XmlParser * xp_init();
extern void xp_delete(XmlParser * tw);
extern void xp_setDocument(XmlParser * tw, const char * str);
extern int xp_parse(XmlParser * tw);
extern int xp_getContentNodesNumber(XmlParser * tw);
extern char * xp_getNodeContent(XmlParser * tw, uint i);
extern void xp_setNodeContent(XmlParser * tw, uint i, const char * str);
extern char * xp_getDocument(XmlParser * tw);
extern void xp_printDoc(XmlParser * tw);

#endif
#ifdef __cplusplus
}
#endif

#endif /* XMLPARSER_H */
