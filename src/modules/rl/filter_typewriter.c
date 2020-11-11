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
    twdata * rendered;      // rndered data
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
    if (cont->rendered)
        tw_delete(cont->rendered->tw);
    free (cont->rendered);
    free (cont->data_field);
    free (cont->xml_data);

    cont->rendered = NULL;
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
 * Find content node in the xml data.
 */
xmlNodePtr find_content_node(xmlDocPtr * doc, const char * d);
/*
 * Get <content></content> node value.
 */
xmlChar * xml_get_content(const char * d);
/*
 * Set <content></content> node value.
 */
xmlChar * xml_set_content(const char * d, const xmlChar * text);

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

            // Get content data and backup in the tw container.
            key = xml_get_content(d);
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

        break;
    }

    if (cont->init != 0)
        return 1;

    if (d == NULL)
        return 0;

    if (update_mask & 0x1)
    {
        twcont_clean(cont);
        twdata * data = twdata_init();

        // save new data field name
        if (cont->data_field) free(cont->data_field);
        cont->data_field = malloc(strlen(data_field)+1);
        strcpy(cont->data_field, data_field);

        // save new xml data
        if (cont->xml_data) free(cont->xml_data);
        cont->xml_data = malloc(strlen(d)+1);
        strcpy(cont->xml_data, d);

        cont->framestep = framestep;
        tw_setPattern(data->tw, (char*)key);
        tw_parse(data->tw);
        cont->rendered = data;

        cont->producer_type = 1;
        cont->producer = producer;
    }
    else if (update_mask & 0x2)
    {
        cont->framestep = framestep;
    }

    // mark as inited
    cont->init = 1;

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
    const char * buff_render = tw_render(cont->rendered->tw, pos/cont->framestep);
    xmlChar * dump = xml_set_content(cont->xml_data, (xmlChar*)buff_render);

    // update producer for rest of the frame
    mlt_properties_set( producer_properties, cont->data_field, (char *)dump );

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
    if ( res == 0)
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


xmlNodePtr find_content_node(xmlDocPtr * doc, const char * d) {
    xmlNodePtr cur;

    // load xml from memory to the model
    *doc = xmlReadMemory(d, strlen(d), "noname.xml", NULL, 0);
    if (*doc == NULL) {
        fprintf(stderr, "Failed to parse document\n");
        return 0;
    }

    cur = xmlDocGetRootElement(*doc);

    if (cur == NULL) {
        fprintf(stderr,"empty document\n");
        xmlFreeDoc(*doc);
        return 0;
    }

    // czeck the top level
    if (xmlStrcmp(cur->name, (const xmlChar *) "kdenlivetitle")) {
        fprintf(stderr,"document of the wrong type, root node != story\n");
        xmlFreeDoc(*doc);
        return 0;
    }

    // serch for item node
    cur = cur->xmlChildrenNode;
    while (cur != NULL) {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"item"))) {
            // search for content node
            cur = cur->xmlChildrenNode;
            while (cur != NULL) {
                if ((!xmlStrcmp(cur->name, (const xmlChar *)"content")))
                    return cur;

                cur = cur->next;
            }
            break;
        }
        cur = cur->next;
    }

    return cur;
}

xmlChar * xml_get_content(const char * d) {
    xmlDocPtr doc = 0;
    xmlNodePtr node = find_content_node(&doc, d);
    xmlChar * buff = 0;
    if (node) {
        xmlChar * key = xmlNodeGetContent(node->xmlChildrenNode);
        buff = malloc(xmlStrlen(key)+1);
        strcpy((char*)buff, (char*)key);
        xmlFree(key);
    }

    return buff;
}

xmlChar * xml_set_content(const char * d, const xmlChar * text) {
    xmlDocPtr doc = 0;
    xmlNodePtr node = find_content_node(&doc, d);
    xmlChar * buff = 0;
    int len;

    if (node) {
        xmlNodeSetContent(node, text);
    }
    xmlDocDumpMemory(doc, &buff, &len);

    xmlFreeDoc(doc);
    return buff;
}
