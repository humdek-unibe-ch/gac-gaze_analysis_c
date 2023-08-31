/**
 * @author  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "gac_screen.h"

/******************************************************************************/
gac_screen_t* gac_screen_create( vec3* top_left, vec3* top_right,
        vec3* bottom_left )
{
    gac_screen_t* screen = malloc( sizeof( gac_screen_t ) );

    if( screen == NULL )
    {
        return NULL;
    }

    if( !gac_screen_init( screen, top_left, top_right, bottom_left ) )
    {
        gac_screen_destroy( screen );
        return NULL;
    }

    screen->_me = screen;

    return screen;
}

/******************************************************************************/
void gac_screen_destroy( gac_screen_t* screen )
{
    if( screen == NULL )
    {
        return;
    }

    if( screen->_me != NULL )
    {
        free( screen->_me );
    }
}

/******************************************************************************/
bool gac_screen_init( gac_screen_t* screen, vec3* top_left, vec3* top_right,
        vec3* bottom_left )
{
    if( screen == NULL || top_left == NULL || top_right == NULL
            || bottom_left == NULL )
    {
        return false;
    }

    gac_plane_init( &screen->plane, top_left, top_right, bottom_left );

    screen->width = glm_vec3_norm( screen->plane.e1 );
    screen->height = glm_vec3_norm( screen->plane.e2 );
    screen->resolution_x = 0;
    screen->resolution_y = 0;
    screen->_me = NULL;
    glm_vec2_zero( screen->origin );

    gac_screen_point( screen, top_left, &screen->origin );

    return true;
}

/******************************************************************************/
bool gac_screen_point( gac_screen_t* screen, vec3* point3d,
        vec2* point2d )
{
    vec2 p_offset, p;

    if( screen == NULL
            || !gac_plane_point( &screen->plane, point3d, &p_offset ) )
    {
        return false;
    }

    glm_vec2_sub( p_offset, screen->origin, p );
    ( *point2d )[0] = p[0] / screen->width;
    ( *point2d )[1] = p[1] / screen->height;

    return true;
}

/******************************************************************************/
bool gac_screen_point_alt( gac_screen_t* screen, vec3* point3d,
        vec2* point2d )
{
    vec2 p;

    if( screen->resolution_x == 0 || screen->resolution_y == 0
            || point2d == NULL )
    {
        return false;
    }

    if( !gac_screen_point( screen, point3d, &p ) )
    {
        return false;
    }

    ( *point2d )[0] = p[0] * screen->resolution_x;
    ( *point2d )[1] = p[1] * screen->resolution_y;

    return true;
}

/******************************************************************************/
bool gac_screen_set_size_alt( gac_screen_t* screen, float resolution_x,
        float resolution_y )
{
    if( screen == NULL )
    {
        return false;
    }

    screen->resolution_x = resolution_x;
    screen->resolution_y = resolution_y;

    return true;
}
