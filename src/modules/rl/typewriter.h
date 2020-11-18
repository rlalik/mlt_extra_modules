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

#ifndef TYPEWRITER_H
#define TYPEWRITER_H

#ifdef __cplusplus

#include <string>
#include <vector>
#include <random>

#include <QDomElement>

using namespace std;

struct Frame
{
    Frame(uint frame, uint real_frame);

    uint frame;
    uint real_frame;
    std::string s;
    int bypass;

    void print();
};

class TypeWriter
{
public:
    TypeWriter();
    virtual ~TypeWriter();

    void setFrameRate(uint fr) { frame_rate = fr; }
    uint getFrameRate() const { return frame_rate; }

    void setFrameStep(uint fs) { frame_step = fs; }
    uint getFrameStep() const { return frame_step; }

    void setStepSigma(float ss) { step_sigma = ss; }
    uint getStepSigma() const { return step_sigma; }

    void setStepSeed(float ss) { step_seed = ss; }
    uint getStepSeed() const { return step_seed; }

    void setPattern(const std::string & str);
    const std::string & getPattern() const { return raw_string; }

    int parse();
    void printParseResult();

    const std::string & render(uint frame);

    uint count() const;
    bool isEnd() const { return last_used_idx == (int)frames.size()-1; }

    void clear();
    void debug() const { for (Frame f : frames) f.print(); }

private:
    int parseString(const std::string & line, int start_frame);

    struct ParseOptions;
    int parseOptions(const std::string& line, uint & i, ParseOptions & po);
    int parseMacro(const std::string& line, uint & i, uint & frame);

    std::string detectUtf8(const std::string & str, size_t pos);

    void insertChar(char c, uint frame);
    void insertString(const std::string & str, uint frame);
    void insertBypass(uint frame);

    uint getFrameSkipFromOptions(const ParseOptions & po, bool steps = false);

    uint getOrInsertFrame(uint frame);
    void addBypass(uint idx);

private:
    size_t frame_rate;
    size_t frame_step;
    float step_sigma;
    size_t step_seed;
    int parsing_err;
    int previous_total_frame;

    std::string raw_string;

    std::vector<Frame> frames;
    int last_used_idx;

    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution<> d;
};

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

private:

private:
    QString doc;
    QDomDocument dom;
    QDomNodeList items;
    std::vector<QDomNode> node_vec;
};

#else
typedef void * TypeWriter;
typedef void * XmlParser;
#endif

#ifdef __cplusplus
extern "C" {
#endif
#if defined(__STDC__) || defined(__cplusplus)

extern TypeWriter * tw_init();
extern void tw_delete(TypeWriter * tw);
extern void tw_setFrameRate(TypeWriter * tw, unsigned int fr);
extern unsigned int tw_getFrameRate(TypeWriter * tw);
extern void tw_setFrameStep(TypeWriter * tw, unsigned int fs);
extern void tw_setStepSigma(TypeWriter * tw, float ss);
extern void tw_setStepSeed(TypeWriter * tw, unsigned int ss);
extern void tw_setPattern(TypeWriter * tw, const char * str);
extern const char * tw_getPattern(TypeWriter * tw);
extern int tw_parse(TypeWriter * tw);
extern void tw_printParseResult(TypeWriter * tw);
extern const char * tw_render(TypeWriter * tw, unsigned int frame);
extern unsigned int tw_count(TypeWriter * tw);
extern unsigned int tw_isEnd(TypeWriter * tw);
extern void tw_clear(TypeWriter * tw);
extern void tw_debug(TypeWriter * tw);

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


#endif /* TYPEWRITER_H */
