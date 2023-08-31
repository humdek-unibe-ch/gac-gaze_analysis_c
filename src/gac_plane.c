/**
 * @author  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "gac_plane.h"

/******************************************************************************/
gac_plane_t* gac_plane_create( vec3* p1, vec3* p2, vec3* p3 )
{
    gac_plane_t* plane = malloc( sizeof( gac_plane_t ) );

    if( plane == NULL )
    {
        return NULL;
    }

    if( !gac_plane_init( plane, p1, p2, p3 ) )
    {
        gac_plane_destroy( plane );
        return NULL;
    }

    plane->_me = plane;

    return plane;
}

/******************************************************************************/
void gac_plane_destroy( gac_plane_t* plane )
{
    if( plane == NULL )
    {
        return;
    }

    if( plane->_me != NULL )
    {
        free( plane->_me );
    }
}

/******************************************************************************/
bool gac_plane_init( gac_plane_t* plane, vec3* p1, vec3* p2, vec3* p3 )
{
    mat4 d = {
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f}
    };
    mat4 s = {
        {0.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 0.0f},
        {1.0f, 1.0f, 1.0f, 1.0f}
    };
    mat4 s_inv;
    vec3 e1, e2, norm;
    vec3 u, v, w;

    if( plane == NULL || p1 == NULL || p2 == NULL || p3 == NULL )
    {
        return false;
    }

    glm_vec3_copy( *p1, plane->p1 );
    glm_vec3_copy( *p2, plane->p2 );
    glm_vec3_copy( *p3, plane->p3 );

    glm_vec3_sub( *p2, *p1, plane->e1 );
    glm_vec3_sub( *p3, *p1, plane->e2 );
    glm_vec3_cross( plane->e1, plane->e2, plane->norm );

    glm_vec3_normalize_to( plane->e1, e1 );
    glm_vec3_normalize_to( plane->e2, e2 );
    glm_vec3_normalize_to( plane->norm, norm );
    glm_vec3_add( *p1, e1, u );
    glm_vec3_add( *p1, e2, v );
    glm_vec3_add( *p1, norm, w );

    s[0][0] = ( *p1 )[0];
    s[1][0] = ( *p1 )[1];
    s[2][0] = ( *p1 )[2];
    s[0][1] = u[0];
    s[1][1] = u[1];
    s[2][1] = u[2];
    s[0][2] = v[0];
    s[1][2] = v[1];
    s[2][2] = v[2];
    s[0][3] = w[0];
    s[1][3] = w[1];
    s[2][3] = w[2];

    glm_mat4_inv( s, s_inv );
    glm_mat4_mul( s_inv, d, plane->m );
    glm_mat4_transpose( plane->m );

    plane->_me = NULL;

    return true;
}

/******************************************************************************/
bool gac_plane_intersection( gac_plane_t* plane, vec3* origin, vec3* dir,
        vec3* intersection )
{
    float d;
    float n;
    vec3 dir_scale, dir_neg, dir_init;

    if( plane == NULL || origin == NULL || dir == NULL
            || intersection == NULL )
    {
        return false;
    }

    glm_vec3_scale( *dir, -1, dir_neg );
    glm_vec3_sub( *origin, plane->p1, dir_init );

    d = glm_vec3_dot( plane->norm, dir_init );
    n = glm_vec3_dot( plane->norm, dir_neg );

    if( n == 0 )
    {
        return false;
    }

    glm_vec3_scale( *dir, d / n, dir_scale );
    glm_vec3_add( *origin, dir_scale, *intersection );

    return true;
}

/******************************************************************************/
bool gac_plane_point( gac_plane_t* plane, vec3* point3d, vec2* point2d )
{
    vec3 p;

    if( plane == NULL || point3d == NULL || point2d == NULL )
    {
        return false;
    }

    glm_mat4_mulv3( plane->m, *point3d, 1, p );
    ( *point2d )[0] = p[0];
    ( *point2d )[1] = p[1];

    return true;
}
