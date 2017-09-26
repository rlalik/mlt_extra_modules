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
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <framework/mlt.h>
#include <framework/mlt_producer.h>
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
    TypeWriter * tw;          // holds TypeWriter object
    int idx_beg;            // index of begin of the pattern
    int idx_end;            // index of the end
} twdata;

typedef struct {
    twdata ** arr;
    int size;
    int limit;
    int init;               // 1 if initialized

    int current_frame;      // currently parsed frame

    char * data_field;      // data field name
    char * data;            // data field content
    char * sbeg;            // begin marker
    char * send;            // end marker

    int producer_type;      // 1 - kdenlivetitle
    mlt_producer producer;  // hold producer pointer
} twcont;

twcont * twcont_clean(twcont* cont)
{
    for (int i = 0; i < cont->size; ++i)
    {
        twdata * data = cont->arr[i];
        tw_delete(data->tw);
        data->tw = NULL;
        free(data);
    }

    free (cont->arr);
    free (cont->data_field);
    free (cont->data);
    free (cont->sbeg);
    free (cont->send);

    cont->arr = NULL;
    cont->size = 0;
    cont->limit = 0;
    cont->init = 0;
    cont->current_frame = -1;
    cont->data_field = NULL;
    cont->data = NULL;
    cont->sbeg = NULL;
    cont->send = NULL;
    cont->producer_type = 0;
    cont->producer = NULL;
    return cont;
}

twcont * twcont_init()
{
    twcont* cont = (twcont*)calloc( 1, sizeof(twcont) );
    twcont_clean(cont);
    return cont;
}

void twcont_resize(twcont * twc) {
    if (twc->size < twc->limit)
        return;

    twc->limit += 10;
    twdata ** arr2 = (twdata**) calloc( twc->limit, sizeof(twdata*) );
    memset(arr2, 0, twc->limit * sizeof(twdata*));
    memcpy(arr2, twc->arr, twc->size * sizeof(twdata*));
    free(twc->arr);
    twc->arr = arr2;
}

twdata * twdata_init()
{
    twdata * data = (twdata*) calloc( 1, sizeof(twdata) );
    data->tw = tw_init();
    data->idx_beg = -1;
    data->idx_end = -1;
    return data;
}

static int get_producer_data(mlt_properties filter_p, mlt_properties frame_p, twcont * cont)
{
    if (cont == NULL)
        return 0;

    char data_field[200];
    char * d = NULL;
    char * str_beg = NULL;
    char * str_end = NULL;

    mlt_producer producer = NULL;
    mlt_properties producer_properties = NULL;

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
            str_beg = mlt_properties_get( filter_p, "beg" );
            str_end = mlt_properties_get( filter_p, "end" );

            int res_d = -1;
            if (cont->data && d)
                res_d = strcmp(cont->data, d);

            int res_sb = -1;
            if (cont->sbeg && str_beg)
                res_sb = strcmp(cont->sbeg, str_beg);

            int res_se = -1;
            if (cont->send && str_end)
                res_se = strcmp(cont->send, str_end);

            if ((res_d != 0) || (res_sb != 0) || (res_se != 0))
            {
                twcont_clean(cont);
            }

            cont->producer_type = 1;
            cont->producer = producer;
            break;
        }

        break;
    }

    if (cont->init != 0)
        return 1;

    if (d == NULL)
        return 0;

    const char * c = d;
    int len_beg = strlen(str_beg);
    int len_end = strlen(str_end);
    int i = 0;
    int i_beg = -1;
    int i_end = -1;

    while (*c != '\0')
    {
        // check first character
        if (*c != str_beg[0])
        {
            ++i;
            ++c;
            continue;
        }

        // check full pattern
        if (strncmp(str_beg, c, len_beg) != 0)
        {
            ++i;
            ++c;
            continue;
        }

        i_beg = i;

        i += len_beg;
        c += len_beg;

        while (*c != '\0')
        {
            // check first character
            if (*c != str_end[0])
            {
                ++i;
                ++c;
                continue;
            }

            // check full pattern
            if (strncmp(str_end, c, len_end) != 0)
            {
                ++i;
                ++c;
                continue;
            }

            i += len_end;
            c += len_end;

            i_end = i;

            break;
        }

        int len = 0;
        char * buff = NULL;
        if (i_beg == -1 || i_end == -1)
        {
            len = 0;
            buff = malloc(len+1);
            memset(buff, 0, len+1);
        }
        else
        {
            len = i_end - i_beg - len_beg - len_end;    // length of pattern w/o markers
            buff = malloc(len+1);
            memset(buff, 0, len+1);
            strncpy(buff, d + i_beg + len_beg, len);

            twdata * data = twdata_init();
            tw_setPattern(data->tw, buff);

            /*int res =*/ tw_parse(data->tw);

            data->idx_beg = i_beg;
            data->idx_end = i_end;

            twcont_resize(cont);
            cont->arr[cont->size] = data;
            ++cont->size;
        }

        cont->sbeg = malloc(len_beg+1);
        cont->send = malloc(len_end+1);
        memset(cont->sbeg, 0, len_beg+1);
        memset(cont->send, 0, len_end+1);

        strcpy(cont->sbeg, str_beg);
        strcpy(cont->send, str_end);

        free(buff);

        i_beg = -1;
        i_end = -1;
    }

    if (cont->data_field) free(cont->data_field);
    cont->data_field = malloc(strlen(data_field)+1);
    strcpy(cont->data_field, data_field);

    if (cont->data) free(cont->data);
    cont->data = malloc(strlen(d)+1);
    strcpy(cont->data, d);

    cont->init = 1;

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
        mlt_properties_set( producer_properties, cont->data_field, cont->data );
        return 1;
    }

    int len_data = strlen(cont->data);
    char * buff_data = malloc(len_data+1);
    if (!buff_data)
    {
        printf("Cannot allocate memory, exiting\n");
        exit(0);
    }
    strcpy(buff_data, cont->data);

    for (int i = cont->size - 1; i >= 0; --i)
    {
        twdata * data = cont->arr[i];
        int len = data->idx_end - data->idx_beg;
        const char * buff_render = tw_render(data->tw, pos);
        int len_render = strlen(buff_render);

        char * tmp_buff = malloc(len_data+1);
        strcpy(tmp_buff, buff_data);
        strncpy(buff_data + data->idx_beg, buff_render, len_render);
        strcpy(buff_data + data->idx_beg + len_render, tmp_buff + data->idx_end);
        free(tmp_buff);
    }
    mlt_properties_set( producer_properties, cont->data_field, buff_data );

    free(buff_data);

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
