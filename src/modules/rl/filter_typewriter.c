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
    TypeWriter * tw;        // holds TypeWriter object
} twdata;

typedef struct {
    XmlParser * xp;

    twdata ** renders;      // rendered data [array]
    int size;               // size of array
    int limit;              // max size of array
    int init;               // 1 if initialized

    int current_frame;      // currently parsed frame

    char * data_field;      // data field name
    char * xml_data;        // data field content (xml data)
    int step_length;        // frame step value
    float sigma;            // sigma of fluctuations
    int seed;               // seed for random fluctuations
    int macro;              // macro type: 0 - custom, 1 - char, 2 - word, 3 - line

    int producer_type;      // 1 - kdenlivetitle
    mlt_producer producer;  // hold producer pointer
} twcont;

twcont * twcont_clean(twcont* cont)
{
    if (cont->xp)
        xp_delete(cont->xp);

    cont->xp = xp_init();

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
    cont->step_length = 0;
    cont->sigma = 0;
    cont->seed = 0;
    cont->macro = 0;
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
 * Get data for display.
 */
static int get_producer_data(mlt_properties filter_p, mlt_properties frame_p, twcont * cont)
{
    if (cont == NULL)
        return 0;

    char data_field[200];
    char * d = NULL;
    int step_length = 0;
    int sigma = 0;
    int seed = 0;
    int macro = 0;

    mlt_producer producer = NULL;
    mlt_properties producer_properties = NULL;

    unsigned int update_mask = 0;

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
            step_length = atoi(mlt_properties_get(filter_p, "step_length"));
            sigma = atof(mlt_properties_get(filter_p, "step_sigma"));
            seed = atoi(mlt_properties_get(filter_p, "random_seed"));
            macro = atoi(mlt_properties_get(filter_p, "macro_type"));

            int res_d = -1;
            if (cont->xml_data && d)
                res_d = strcmp(cont->xml_data, d);

            // if xml data changed, set update mask 0x1
            if (res_d != 0 || macro != cont->macro)
                update_mask = 0x3;

            if (step_length != cont->step_length || sigma != cont->sigma || seed != cont->seed)
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

        // Get content data and backup in the tw container.
        xp_setDocument(cont->xp, d);
        xp_parse(cont->xp);
        unsigned int n = xp_getContentNodesNumber(cont->xp);

        for (int i = 0; i < n; ++i)
        {
            char * key = xp_getNodeContent(cont->xp, i);
            twdata * data = twdata_init();

            if (macro) {
                char * buff = malloc(strlen(key)+5);
                char c = 0;
                switch (macro) {
                    case 1: c = 'c'; break;
                    case 2: c = 'w'; break;
                    case 3: c = 'l'; break;
                    default: break;
                }

                sprintf(buff, ":%c{%s}", c, key);
                tw_setPattern(data->tw, buff);
                free(buff);
            } else {
                tw_setPattern(data->tw, key);
            }
            tw_setFrameStep(data->tw, step_length);
            tw_setStepSigma(data->tw, sigma);
            tw_setStepSeed(data->tw, seed);
            tw_parse(data->tw);

            twcont_resize(cont);
            cont->renders[cont->size] = data;
            ++cont->size;
            free(key);
        }

        cont->macro = macro;
        cont->producer_type = 1;
        cont->producer = producer;

        // mark as inited
        cont->init = 1;
    }
    else if (update_mask & 0x2)
    {
        for (int i = 0; i < cont->size; ++i)
        {
            tw_setFrameStep(cont->renders[i]->tw, step_length);
            tw_setStepSigma(cont->renders[i]->tw, sigma);
            tw_setStepSeed(cont->renders[i]->tw, seed);
            tw_parse(cont->renders[i]->tw);
        }
        cont->step_length = step_length;
        cont->sigma = sigma;
        cont->seed = seed;
    }

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
    unsigned int n = xp_getContentNodesNumber(cont->xp);
    for (int i = 0; i < n; ++i) {
        const char * buff_render = tw_render(cont->renders[i]->tw, pos);
        xp_setNodeContent(cont->xp, i, buff_render);
    }

    // update producer for rest of the frame
    char * dom = xp_getDocument(cont->xp);
    mlt_properties_set( producer_properties, cont->data_field, dom );
    free(dom);

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
