/*
 * factory.c -- the factory method interfaces
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

#include <string.h>
#include <framework/mlt.h>
#include <limits.h>

extern mlt_filter filter_typewriter_init( mlt_profile profile, mlt_service_type type, const char *id, char *arg );

static mlt_properties typewriter_metadata( mlt_service_type type, const char *id, void *data )
{
    char file[ PATH_MAX ];
    snprintf( file, PATH_MAX, "%s/rl/filter_%s.yml", mlt_environment( "MLT_DATA" ), id );
    return mlt_properties_parse_yaml( file );
}

MLT_REPOSITORY
{
    MLT_REGISTER( filter_type, "typewriter", filter_typewriter_init );

    MLT_REGISTER_METADATA( filter_type, "typewriter", typewriter_metadata, NULL );
}
