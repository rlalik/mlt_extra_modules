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

#include "typewriter.h"
#include <string>
#include <sstream>

using namespace std;

std::string null_string;

TypeWriter::TypeWriter() : frame_rate(25), parsing_err(0), last_used_idx(-1)
{
}

TypeWriter::~TypeWriter()
{
}

void TypeWriter::clear()
{
    frames.clear();
}

void TypeWriter::setPattern(const std::string& str)
{
    raw_string = str;
    frames.reserve(raw_string.length());
}

int TypeWriter::parse()
{
    clear();

    int start_frame = 0;
    parsing_err = parseString(raw_string, start_frame);
    return parsing_err;
}

void TypeWriter::printParseResult()
{
    if (parsing_err < 0)
    {
        fprintf(stderr, "Parsing error:\n%.*s\n", -parsing_err-1, raw_string.c_str());
        fprintf(stderr, "%*c%c\n", -parsing_err-2, ' ', '^');
    }
    else
    {
        printf("Parsing OK:  frames=%u  strings=%ld\n", count(), frames.size());
    }
}

uint TypeWriter::count() const
{
    return frames.back().frame;
}


uint TypeWriter::getOrInsertFrame(uint frame)
{
    // create new or reuse old frame
    // by design last->frame cannot be larger than frame
    // take the last frame then FIXME: should we break parser here?

    uint n = frames.size();
    if (!n)
    {
        frames.push_back(Frame(frame));
        return 0;
    }

    if (frames[n-1].frame >= frame)
        return n-1;

    Frame f = Frame(frame);
    f.s = frames[n-1].s;
    frames.push_back(f);

    return n;
}

void TypeWriter::insertChar(char c, uint frame)
{
    char buff[2] = "\0";
    buff[0] = c;
    insertString(buff, frame);
}

void TypeWriter::insertString(const std::string & str, uint frame)
{
    uint n = getOrInsertFrame(frame);
    Frame & f = frames[n];
    f.s.append(str);
}

void TypeWriter::insertBypass(uint frame)
{
    uint n = getOrInsertFrame(frame);

    addBypass(n);
}

const std::string & TypeWriter::render(uint frame)
{
    uint n = frames.size();
    if (!n)
        return null_string;

    if (last_used_idx == -1)
        last_used_idx = 0;

    // start with current frame
    Frame f = frames[last_used_idx];

    // but if current is ahead 'frame', start from beginning
    if (f.frame > frame)
        last_used_idx = 0;

    if (frames[last_used_idx].frame > frame)
        return null_string;

    for (; last_used_idx < (int)n-1; ++last_used_idx)
    {
        f = frames[last_used_idx+1];
        if (f.frame > frame)
            return frames[last_used_idx].s;
    }

    return frames[last_used_idx].s;
}

void TypeWriter::addBypass(uint idx)
{
    if (idx == 0)
    {
        frames[idx].s.clear();
        return;
    }

    int pidx = -1;

    if (frames[idx].bypass == -2)
        pidx = idx-1;
    else
        pidx = frames[idx].bypass;

    if (pidx == -1)
        return;

    while (true)
    {
        if (frames[pidx].bypass != -2)
        {
            pidx = frames[pidx].bypass;
        }
        else
        {
            --pidx;
            break;
        }
    }
    frames[idx].bypass = pidx;

    if (frames[idx].bypass >= 0)
        frames[idx].s = frames[frames[idx].bypass].s;
    else
        frames[idx].s.clear();
}

Frame::Frame(uint frame) : frame(frame), bypass(-2)
{
}

void Frame::print()
{
    printf("%c [%d] %s %c\n",
           true ? '-' : '|',
           frame, s.c_str(),
           true ? '-' : '|');
}

std::string TypeWriter::detectUtf8(const std::string& str, size_t pos)
{
    /*
     * 0x00 do 0x7F            – bits 0xxxxxxx
     * 0x80 do 0x7FF           – bits 110xxxxx 10xxxxxx
     * 0x800 do 0xFFFF         – bits 1110xxxx 10xxxxxx 10xxxxxx
     * 0x10000 do 0x1FFFFF     – bits 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
     * 0x200000 do 0x3FFFFFF   – bits 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
     * 0x4000000 do 0x7FFFFFFF – bits 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
     */
    unsigned char c = str[pos];
    static const unsigned char masks[5] = { 0xfc, 0xf8, 0xf0, 0xc0, 0x80 };
    if ((c & 0x80) == 0x00)
    {
        return str.substr(pos, 1);
    }
    else
    {
        for (int i = 0; i < 5; ++i)
        {
            if ((c & masks[i]) == masks[i])
            {
                for (int j = 0; j < (4-i); ++j)
                {
                    if ( !(0x80 & str[pos+1+j]) )
                        return str.substr(pos, 1);
                }
                return str.substr(pos, 5-i);
            }
        }
    }
    return "";
}
