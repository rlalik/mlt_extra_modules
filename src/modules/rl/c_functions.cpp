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

#include <cstring>

TypeWriter * tw_init()
{
    TypeWriter *tw = new TypeWriter;
    return tw;
}

void tw_delete(TypeWriter * tw)
{
    delete (TypeWriter *) tw;
    tw = 0;
}

void tw_setFrameRate(TypeWriter * tw, unsigned int fr)
{
    tw->setFrameRate(fr);
}

unsigned int tw_getFrameRate(TypeWriter * tw)
{
    return tw->getFrameRate();
}

void tw_setPattern(TypeWriter * tw, const char * str)
{
    tw->setPattern(str);
}

const char * tw_getPattern(TypeWriter * tw)
{
    return tw->getPattern().c_str();
}

int tw_parse(TypeWriter * tw)
{
    return tw->parse();
}

void tw_printParseResult(TypeWriter * tw)
{
    return tw->printParseResult();
}

const char * tw_render(TypeWriter * tw, unsigned int frame)
{
    return tw->render(frame).c_str();
}

unsigned int tw_count(TypeWriter * tw)
{
    return tw->count();
}

unsigned int tw_isEnd(TypeWriter * tw)
{
    return tw->isEnd();
}

void tw_clear(TypeWriter * tw)
{
    tw->clear();
}

void tw_debug(TypeWriter * tw)
{
    tw->debug();
}
