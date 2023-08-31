/**
 * @author  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "gac_aoi.h"

/******************************************************************************/
bool gac_aoi_add_point( gac_aoi_t* aoi, float x, float y )
{
    float distance;

    if( aoi == NULL || aoi->count == GAC_AOI_MAX_POINTS )
    {
        return false;
    }

    aoi->points[aoi->count][0] = x;
    aoi->points[aoi->count][1] = y;

    if( aoi->count > 0 )
    {
        distance = glm_vec2_distance( aoi->points[aoi->count],
                aoi->points[aoi->count - 1] );
        aoi->avg_edge_len += ( distance - aoi->avg_edge_len ) / aoi->count;
    }

    aoi->count++;

    if( x < aoi->ray_origin[0] )
    {
        aoi->ray_origin[0] = x - aoi->avg_edge_len;
    }

    if( y < aoi->ray_origin[1] )
    {
        aoi->ray_origin[1] = y - aoi->avg_edge_len;
    }

    return true;
}

/******************************************************************************/
gac_aoi_t* gac_aoi_create()
{
    gac_aoi_t* aoi = malloc( sizeof( gac_aoi_t ) );

    if( !gac_aoi_init( aoi ) )
    {
        return NULL;
    }
    aoi->_me = aoi;

    return aoi;
}

/******************************************************************************/
void gac_aoi_destroy( gac_aoi_t* aoi )
{
    if( aoi == NULL )
    {
        return;
    }

    if( aoi->_me != NULL )
    {
        free( aoi->_me );
    }
}

/******************************************************************************/
bool gac_aoi_includes_point( gac_aoi_t* aoi, float x, float y )
{
    uint32_t i;
    uint32_t count = 0;
    vec2 point = { x, y };


    if( aoi == NULL && aoi->count < 3 )
    {
        return false;
    }

    for( i = 1; i < aoi->count; i++ )
    {
        if( gac_aoi_intersect( &aoi->ray_origin, &point, &aoi->points[i - 1],
                &aoi->points[i] ) )
        {
            count++;
        }
    }

    if( count % 2 == 0 )
    {
        return false;
    }
    else
    {
        return true;
    }

}

/******************************************************************************/
bool gac_aoi_init( gac_aoi_t* aoi )
{
    uint32_t i;

    if( aoi == NULL )
    {
        return false;
    }

    glm_vec2_zero( aoi->ray_origin );
    aoi->count = 0;
    aoi->avg_edge_len = 0;
    aoi->_me = NULL;

    for( i = 0; i < GAC_AOI_MAX_POINTS; i++ )
    {
        glm_vec2_zero( aoi->points[i] );
    }

    return false;
}

/******************************************************************************/
bool gac_aoi_intersect( vec2* p1, vec2* q1, vec2* p2, vec2* q2 )
{
    gac_aoi_orientation_t o1 = gac_aoi_orientation_triplet( p1, q1, p2 );
    gac_aoi_orientation_t o2 = gac_aoi_orientation_triplet( p1, q1, q2 );
    gac_aoi_orientation_t o3 = gac_aoi_orientation_triplet( p2, q2, p1 );
    gac_aoi_orientation_t o4 = gac_aoi_orientation_triplet( p2, q2, q1 );

    // General case
    if( o1 != o2 && o3 != o4 )
    {
        return true;
    }

    // Special Cases
    if( o1 == GAC_AOI_ORIENTATION_COLINEAR
            && gac_aoi_point_on_segment( p1, p2, q1 ) )
    {
        // p1, q1 and p2 are collinear and p2 lies on segment p1q1
        return true;
    }

    if( o2 == GAC_AOI_ORIENTATION_COLINEAR
            && gac_aoi_point_on_segment( p1, q2, q1 ) )
    {
        // p1, q1 and q2 are collinear and q2 lies on segment p1q1
        return true;
    }

    if( o3 == GAC_AOI_ORIENTATION_COLINEAR
            && gac_aoi_point_on_segment( p2, p1, q2 ) )
    {
        // p2, q2 and p1 are collinear and p1 lies on segment p2q2
        return true;
    }

    if( o4 == GAC_AOI_ORIENTATION_COLINEAR
            && gac_aoi_point_on_segment( p2, q1, q2 ) )
    {
        // p2, q2 and q1 are collinear and q1 lies on segment p2q2
        return true;
    }

    return false;
}

/******************************************************************************/
bool gac_aoi_point_on_segment( vec2* p, vec2* s1, vec2* s2 )
{
    if( ( *p )[0] <= fmax( ( *s1 )[0], ( *s2 )[0] )
            && ( *p )[0] >= fmin( ( *s1 )[0], ( *s2 )[0] )
            && ( *p )[1] <= fmax( ( *s1 )[1], ( *s2 )[1] )
            && ( *p )[1] >= fmin( ( *s1 )[1], ( *s2 )[1] ) )
    {
        return true;
    }

    return false;
}

/******************************************************************************/
gac_aoi_orientation_t gac_aoi_orientation_triplet( vec2* p, vec2* q, vec2* r )
{
    uint32_t val = ( ( *q )[1] - ( *p )[1] ) * ( ( *r )[0] - ( *q )[0] ) -
            ( ( *q )[0] - ( *p )[0]) * ( ( *r )[1] - ( *q )[1] );

    if( val == 0 )
    {
        return GAC_AOI_ORIENTATION_COLINEAR;
    }
    else if( val > 0 )
    {
        return GAC_AOI_ORIENTATION_CLOCKWISE;
    }
    else
    {
        return GAC_AOI_ORIENTATION_COUNTER_CLOCKWISE;
    }
}
