/*
 * filter_oldfilm.c -- oldfilm filter
 * Copyright (c) 2007 Marco Gittler <g.marco@freenet.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied wrenderedanty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <framework/mlt.h>
#include <framework/mlt_filter.h>
#include <framework/mlt_frame.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "typewriter.h"

struct producer_ktitle_s
{
    struct mlt_producer_s parent;
    uint8_t *rgba_image;
    uint8_t *current_image;
    uint8_t *current_alpha;
    mlt_image_format format;
    int current_width;
    int current_height;
    int has_alpha;
    pthread_mutex_t mutex;
};

typedef struct producer_ktitle_s *producer_ktitle;

typedef struct {
    TypeWriter * tw;          // holds TypeWriter object
} twdata;

typedef struct {
    twdata ** renders;      // rendered data [array]
    int size;               // size of array
    int limit;              // max size of array
    int init;               // 1 if initialized

    int current_frame;      // currently parsed frame

    char * data_field;      // data field name
    char * xml_data;        // data field content (xml data)
    int framestep;

    int producer_type;      // 1 - kdenlivetitle
    mlt_producer producer;  // hold producer pointer
} twcont;

twcont * twcont_clean(twcont* cont)
{
    for (int i = 0; i < cont->size; ++i)
    {
        twdata * data = cont->renders[i];
        tw_delete(data->tw);
        data->tw = NULL;
        free(data);
    }

    free (cont->renders);
    free (cont->data_field);
    free (cont->xml_data);

    cont->renders = NULL;
    cont->size = 0;
    cont->limit = 0;
    cont->init = 0;
    cont->current_frame = -1;
    cont->data_field = NULL;
    cont->xml_data = NULL;
    cont->framestep = 0;
    cont->producer_type = 0;
    cont->producer = NULL;
    return cont;
}

/*
 * Init the tw container.
 */
twcont * twcont_init()
{
    twcont* cont = (twcont*)calloc( 1, sizeof(twcont) );
    twcont_clean(cont);
    return cont;
}

/*
 * Init the tw data.
 */
twdata * twdata_init()
{
    twdata * data = (twdata*) calloc( 1, sizeof(twdata) );
    data->tw = tw_init();
    return data;
}

/*
 * Resize the renders array.
 */
void twcont_resize(twcont * twc) {
    if (twc->size < twc->limit)
        return;

    twc->limit += 10;
    twdata ** arr2 = (twdata**) calloc( twc->limit, sizeof(twdata*) );
    memset(arr2, 0, twc->limit * sizeof(twdata*));
    memcpy(arr2, twc->renders, twc->size * sizeof(twdata*));
    free(twc->renders);
    twc->renders = arr2;
}

/*
 * Find content node in the xml data.
 */
xmlNodePtr find_content_node(xmlDocPtr * doc, xmlNodePtr cur, const char * d, int from_begining);
/*
 * Get <content></content> node value.
 */
xmlChar * xml_get_content(xmlDocPtr * doc, const char * d, int from_begining);
/*
 * Set <content></content> node value.
 */
int xml_set_content(xmlDocPtr * doc, const char * d, const xmlChar * text, int from_begining);

/*
 * Get data for display.
 */
static int get_producer_data(mlt_properties filter_p, mlt_properties frame_p, twcont * cont)
{
    if (cont == NULL)
        return 0;

    char data_field[200];
    char * d = NULL;
    int framestep = 0;

    mlt_producer producer = NULL;
    mlt_properties producer_properties = NULL;

    xmlChar *key = 0;

    uint update_mask = 0;

    // fake loop, break after one loop
    while (1)
    {
        /* Try with kdenlivetitle */
        producer_ktitle kt = mlt_properties_get_data( frame_p, "producer_kdenlivetitle", NULL );
        if (kt != NULL)
        {
            /* Obtain properties of producer */
            producer = &kt->parent;
            producer_properties = MLT_PRODUCER_PROPERTIES( producer );

            if (producer == NULL || producer_properties == NULL)
                return 0;

            strcpy(data_field, "xmldata");
            d = mlt_properties_get( producer_properties, data_field );
            framestep = atoi(mlt_properties_get(filter_p, "framestep"));

            int res_d = -1;
            if (cont->xml_data && d)
                res_d = strcmp(cont->xml_data, d);

            // if xml data changed, set update mask 0x1
            if (res_d != 0)
                update_mask = 0x1;

            if (framestep != cont->framestep)
                update_mask |= 0x2;

            // clear and prepare for new parsing
            if (0 == update_mask)
                return 1;
            break;
        }
    }

    if (update_mask & 0x1)
    {
        twcont_clean(cont);

        // save new data field name
        if (!cont->data_field || strlen(cont->data_field) < strlen(data_field))
        {
            if (cont->data_field) free(cont->data_field);
            cont->data_field = malloc(strlen(data_field)+1);
        }
        strcpy(cont->data_field, data_field);

        // save new xml data
        if (!cont->xml_data || strlen(cont->xml_data) < strlen(d))
        {
            if (cont->xml_data) free(cont->xml_data);
            cont->xml_data = malloc(strlen(d)+1);
        }
        strcpy(cont->xml_data, d);

        cont->framestep = framestep;

        // Get content data and backup in the tw container.
        xmlDocPtr doc;
        while ((key = xml_get_content(&doc, d, (0 == cont->size))))
        {
            twdata * data = twdata_init();
            tw_setPattern(data->tw, (char*)key);
            tw_parse(data->tw);

            twcont_resize(cont);
            cont->renders[cont->size] = data;
            ++cont->size;
        }
        xml_get_content(NULL, NULL, -1);

        cont->producer_type = 1;
        cont->producer = producer;

        // mark as inited
        cont->init = 1;
    }
    else if (update_mask & 0x2)
    {
        cont->framestep = framestep;
    }

    xmlFree(key);
    return 1;
}

static int update_producer(mlt_frame frame, mlt_properties frame_p, twcont * cont, int restore)
{
    if (cont->init == 0)
        return 0;

    mlt_position pos = mlt_frame_original_position( frame );

    mlt_properties producer_properties = NULL;
    if (cont->producer_type == 1)
    {
        producer_properties = MLT_PRODUCER_PROPERTIES( cont->producer );
        if (restore)
            mlt_properties_set_int( producer_properties, "force_reload", 0 );
        else
            mlt_properties_set_int( producer_properties, "force_reload", 1 );
    }

    if (producer_properties == NULL)
        return 0;

    if (restore == 1)
    {
        mlt_properties_set( producer_properties, cont->data_field, cont->xml_data );
        return 1;
    }

    // render the string and set as a content value
    xmlDocPtr doc;
    for (int i = 0; i < cont->size; ++i) {
        const char * buff_render = tw_render(cont->renders[i]->tw, pos/cont->framestep);
        int res = xml_set_content(&doc, cont->xml_data, (xmlChar*)buff_render, !i);
        if (0 == res)
            break;
    }
    xml_set_content(NULL, NULL, NULL, -1);

    xmlChar * dump = NULL;
    int len;
    xmlDocDumpMemory(doc, &dump, &len);
    // update producer for rest of the frame
    mlt_properties_set( producer_properties, cont->data_field, (char *)dump );
    xmlFreeDoc(doc);

    cont->current_frame = pos;

    return 1;
}

static int filter_get_image( mlt_frame frame, uint8_t **image, mlt_image_format *format, int *width, int *height, int writable )
{
    int error = 0;
    mlt_filter filter = (mlt_filter) mlt_frame_pop_service( frame );
    mlt_properties frame_properties = MLT_FRAME_PROPERTIES( frame );

    mlt_properties properties = MLT_FILTER_PROPERTIES( filter );

    twcont * cont = (twcont*) filter->child;

    int res = get_producer_data(properties, frame_properties, cont);
    if (res == 0)
        return mlt_frame_get_image( frame, image, format, width, height, 1 );

    update_producer(frame, frame_properties, cont, 0);

    error = mlt_frame_get_image( frame, image, format, width, height, 1 );

    update_producer(frame, frame_properties, cont, 1);

    return error;
}

static mlt_frame filter_process( mlt_filter filter, mlt_frame frame )
{
    mlt_frame_push_service( frame, filter );
    mlt_frame_push_get_image( frame, filter_get_image );
    return frame;
}

static void filter_close( mlt_filter filter)
{
    twcont * cont = filter->child;

    twcont_clean(cont);
}

mlt_filter filter_typewriter_init( mlt_profile profile, mlt_service_type type, const char *id, char *arg )
{
    mlt_filter filter = mlt_filter_new( );
    twcont* cont = twcont_init();

    if ( filter != NULL && cont != NULL)
    {
        filter->process = filter_process;
        filter->child = cont;
        filter->close = filter_close;
    }
    return filter;
}


xmlNodePtr find_content_node(xmlDocPtr * doc, xmlNodePtr cur, const char * d, int from_begining) {
    // load xml from memory to the model
    if (from_begining)
    {
        *doc = xmlReadMemory(d, strlen(d), "noname.xml", NULL, 0);
        if (*doc == NULL) {
            fprintf(stderr, "Failed to parse document\n");
            return NULL;
        }

        xmlFreeNode(cur);
        cur = xmlDocGetRootElement(*doc);

        if (cur == NULL) {
            fprintf(stderr,"empty document\n");
            return NULL;
        }

        // czeck the top level
        if (xmlStrcmp(cur->name, (const xmlChar *) "kdenlivetitle")) {
            fprintf(stderr,"document of the wrong type, root node != story\n");
            return NULL;
        }

        // serch for item node
        cur = cur->children;
    }

    while (cur != NULL) {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"item"))) {
            // search for content node
            cur = cur->children;
            while (cur != NULL) {
                if ((!xmlStrcmp(cur->name, (const xmlChar *)"content")))
                    return cur;

                cur = cur->next;
            }
        }
        else
        {
            cur = cur->next;
        }
    }

    return cur;
}

xmlChar * xml_get_content(xmlDocPtr * doc, const char * d, int from_begining) {
    static xmlNodePtr node = NULL;

    if (from_begining < 0)
    {
        node = NULL;
        return NULL;
    }

    node = find_content_node(doc, node, d, from_begining || !node);

    if (NULL == node)
        return NULL;

    xmlChar * buff = NULL;
    if (node) {
        buff = xmlNodeGetContent(node->xmlChildrenNode);
        node = node->parent->next;
    }

    return buff;
}

int xml_set_content(xmlDocPtr * doc, const char * d, const xmlChar * text, int from_begining) {
    static xmlNodePtr node = NULL;

    if (from_begining < 0)
    {
        node = NULL;
        return 0;
    }

    node = find_content_node(doc, node, d, from_begining || !node);

    if (NULL == node)
        return 0;

    if (node) {
        xmlNodeSetContent(node, text);
        node = node->parent->next;
    }

    return 1;
}
