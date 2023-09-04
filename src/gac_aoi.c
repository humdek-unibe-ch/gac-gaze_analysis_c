/**
 * @author  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "gac_aoi.h"
#include <string.h>

/******************************************************************************/
bool gac_aoi_add_point( gac_aoi_t* aoi, float x, float y )
{
    float distance;

    if( aoi == NULL || aoi->points.count == GAC_AOI_MAX_POINTS )
    {
        return false;
    }

    aoi->points.items[aoi->points.count][0] = x;
    aoi->points.items[aoi->points.count][1] = y;

    if( x < aoi->bounding_box.x_min )
    {
        aoi->bounding_box.x_min = x;
    }
    if( x > aoi->bounding_box.x_max )
    {
        aoi->bounding_box.x_max = x;
    }
    if( y < aoi->bounding_box.y_min )
    {
        aoi->bounding_box.y_min = y;
    }
    if( y > aoi->bounding_box.y_max )
    {
        aoi->bounding_box.y_max = y;
    }

    if( aoi->points.count > 0 )
    {
        distance = glm_vec2_distance( aoi->points.items[aoi->points.count],
                aoi->points.items[aoi->points.count - 1] );
        aoi->avg_edge_len +=
            ( distance - aoi->avg_edge_len ) / aoi->points.count;

        if( x <= aoi->ray_origin[0] )
        {
            aoi->ray_origin[0] = x - aoi->avg_edge_len;
        }

        if( y <= aoi->ray_origin[1] )
        {
            aoi->ray_origin[1] = y - aoi->avg_edge_len;
        }
    }

    aoi->points.count++;

    return true;
}

/******************************************************************************/
bool gac_aoi_add_point_res( gac_aoi_t* aoi, float x_res, float y_res )
{
    float x, y;

    if( aoi->resolution_x == 0 || aoi->resolution_y == 0 )
    {
        return false;
    }

    x =  x_res / aoi->resolution_x;
    y =  y_res / aoi->resolution_y;

    return gac_aoi_add_point( aoi, x, y );
}

/******************************************************************************/
bool gac_aoi_add_rect( gac_aoi_t* aoi, float x, float y, float width,
        float height )
{
    bool res = true;
    res &= gac_aoi_add_point( aoi, x, y );
    res &= gac_aoi_add_point( aoi, x + width, y );
    res &= gac_aoi_add_point( aoi, x + width, y + height );
    res &= gac_aoi_add_point( aoi, x, y + height );

    return true;
}

/******************************************************************************/
bool gac_aoi_add_rect_res( gac_aoi_t* aoi, float x, float y, float width,
        float height )
{
    bool res = true;
    res &= gac_aoi_add_point_res( aoi, x, y );
    res &= gac_aoi_add_point_res( aoi, x + width, y );
    res &= gac_aoi_add_point_res( aoi, x + width, y + height );
    res &= gac_aoi_add_point_res( aoi, x, y + height );

    return true;
}

/******************************************************************************/
gac_aoi_t* gac_aoi_copy( gac_aoi_t* aoi )
{
    gac_aoi_t* aoi_copy;

    if( aoi == NULL )
    {
        return aoi;
    }

    aoi_copy = gac_aoi_create( aoi->label );
    if( aoi_copy == NULL )
    {
        return NULL;
    }

    if( !gac_aoi_copy_to( aoi_copy, aoi ) )
    {
        gac_aoi_destroy( aoi_copy );
        return NULL;
    }

    return aoi_copy;
}

/******************************************************************************/
bool gac_aoi_copy_to( gac_aoi_t* tgt, gac_aoi_t* src )
{
    uint32_t i;
    bool res = true;

    if( tgt == NULL || src == NULL )
    {
        return false;
    }

    res &= gac_aoi_init( tgt, src->label );
    res &= gac_aoi_analysis_copy_to( &tgt->analysis, &src->analysis );
    for( i = 0; i < src->points.count; i++ )
    {
        res &= gac_aoi_add_point( tgt, src->points.items[i][0],
                src->points.items[i][1] );
    }
    res &= gac_aoi_set_resolution( tgt, src->resolution_x, src->resolution_y );

    return res;
}

/******************************************************************************/
gac_aoi_t* gac_aoi_create( const char* label )
{
    gac_aoi_t* aoi = malloc( sizeof( gac_aoi_t ) );

    if( !gac_aoi_init( aoi, label ) )
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

    gac_aoi_analysis_destroy( &aoi->analysis );

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

    if( aoi == NULL && aoi->points.count < 3 )
    {
        return false;
    }

    if( x < aoi->bounding_box.x_min || x > aoi->bounding_box.x_max
            || y < aoi->bounding_box.y_min || y > aoi->bounding_box.y_max )
    {
        // point is outside bounding box, hence it cannot be inside the aoi.
        return false;
    }

    for( i = 1; i < aoi->points.count; i++ )
    {
        if( gac_aoi_intersect( &aoi->ray_origin, &point,
                    &aoi->points.items[i - 1], &aoi->points.items[i] ) )
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
bool gac_aoi_includes_point_res( gac_aoi_t* aoi, float x_res, float y_res )
{
    float x, y;

    if( aoi->resolution_x == 0 || aoi->resolution_y == 0 )
    {
        return false;
    }

    x =  x_res / aoi->resolution_x;
    y =  y_res / aoi->resolution_y;

    return gac_aoi_includes_point( aoi, x, y );
}

/******************************************************************************/
bool gac_aoi_init( gac_aoi_t* aoi, const char* label )
{
    uint32_t i;

    if( aoi == NULL )
    {
        return false;
    }

    aoi->ray_origin[0] = INFINITY;
    aoi->ray_origin[1] = INFINITY;
    aoi->bounding_box.x_max = -INFINITY;
    aoi->bounding_box.x_min = INFINITY;
    aoi->bounding_box.y_max = -INFINITY;
    aoi->bounding_box.y_min = INFINITY;
    aoi->resolution_x = 0;
    aoi->resolution_y = 0;
    aoi->points.count = 0;
    aoi->avg_edge_len = 0;
    aoi->_me = NULL;
    memset( aoi->label, '\0', sizeof( aoi->label ) );
    if( label != NULL )
    {
        strncpy( aoi->label, label, GAC_AOI_MAX_LABEL_LEN - 1 );
    }
    gac_aoi_analysis_init( &aoi->analysis );

    for( i = 0; i < GAC_AOI_MAX_POINTS; i++ )
    {
        glm_vec2_zero( aoi->points.items[i] );
    }

    return true;
}

/******************************************************************************/
bool gac_aoi_intersect( vec2* p1, vec2* q1, vec2* p2, vec2* q2 )
{
    gac_aoi_orientation_t o1 = gac_aoi_orientation_triplet( q1, p1, p2 );
    gac_aoi_orientation_t o2 = gac_aoi_orientation_triplet( q1, p1, q2 );
    gac_aoi_orientation_t o3 = gac_aoi_orientation_triplet( q2, p2, p1 );
    gac_aoi_orientation_t o4 = gac_aoi_orientation_triplet( q2, p2, q1 );

    // General case
    if( o1 != o2 && o3 != o4 )
    {
        return true;
    }

    // Special Cases
    if( o1 == GAC_AOI_ORIENTATION_COLINEAR
            && gac_aoi_point_on_segment( p2, p1, q1 ) )
    {
        // p1, q1 and p2 are collinear and p2 lies on segment p1q1
        return true;
    }

    if( o2 == GAC_AOI_ORIENTATION_COLINEAR
            && gac_aoi_point_on_segment( q2, p1, q1 ) )
    {
        // p1, q1 and q2 are collinear and q2 lies on segment p1q1
        return true;
    }

    if( o3 == GAC_AOI_ORIENTATION_COLINEAR
            && gac_aoi_point_on_segment( p1, p2, q2 ) )
    {
        // p2, q2 and p1 are collinear and p1 lies on segment p2q2
        return true;
    }

    if( o4 == GAC_AOI_ORIENTATION_COLINEAR
            && gac_aoi_point_on_segment( q1, p2, q2 ) )
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

/******************************************************************************/
bool gac_aoi_set_resolution( gac_aoi_t* aoi, float resolution_x,
        float resolution_y )
{
    if( aoi == NULL )
    {
        return false;
    }

    aoi->resolution_x = resolution_x;
    aoi->resolution_y = resolution_y;

    return true;
}
