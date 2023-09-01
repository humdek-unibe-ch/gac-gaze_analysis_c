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
    uint32_t i;
    gac_aoi_t* aoi_copy;

    if( aoi == NULL )
    {
        return aoi;
    }

    aoi_copy = gac_aoi_create( aoi->label );
    aoi_copy->analysis = gac_aoi_analysis_copy( aoi->analysis );
    for( i = 0; i < aoi->points.count; i++ )
    {
        gac_aoi_add_point( aoi_copy, aoi->points.items[i][0],
                aoi->points.items[i][1] );
    }
    gac_aoi_set_resolution( aoi_copy, aoi->resolution_x, aoi->resolution_y );

    return aoi_copy;
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

    if( aoi->label != NULL )
    {
        free( aoi->label );
    }

    if( aoi->analysis != NULL )
    {
        gac_aoi_analysis_destroy( aoi->analysis );
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
    aoi->analysis = NULL;
    aoi->label = NULL;
    if( label != NULL )
    {
        aoi->label = strdup( label );
    }

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

/******************************************************************************/
bool gac_aoi_collection_add( gac_aoi_collection_t* aoic, gac_aoi_t* aoi )
{
    return gac_aoi_collection_assign( aoic, gac_aoi_copy( aoi ) );
}

/******************************************************************************/
bool gac_aoi_collection_analyse_finalise( gac_aoi_collection_t* aoic )
{
    uint32_t i;
    gac_aoi_t* aoi;
    if( aoic == NULL )
    {
        return false;
    }

    if( aoic->analysis == NULL )
    {
        return false;
    }

    for( i = 0; i < aoic->aois.count; i++ )
    {
        aoi = aoic->aois.items[i];
        if( aoi->analysis != NULL )
        {
            aoi->analysis->fixation_count_relative =
                ( double )aoi->analysis->fixation_count /
                    ( double )aoic->analysis->fixation_count;
            aoi->analysis->dwell_time_relative =
                aoi->analysis->dwell_time / aoic->analysis->dwell_time;
        }
    }

    return true;
}

/******************************************************************************/
bool gac_aoi_collection_analyse_fixation( gac_aoi_collection_t* aoic,
        gac_fixation_t* fixation )
{
    uint32_t i;
    gac_aoi_t* aoi;
    if( aoic == NULL || fixation == NULL )
    {
        return false;
    }

    if( aoic->analysis == NULL )
    {
        aoic->analysis = gac_aoi_collection_analysis_create();
    }
    aoic->analysis->dwell_time += fixation->duration;

    for( i = 0; i < aoic->aois.count; i++ )
    {
        aoi = aoic->aois.items[i];
        if( gac_aoi_includes_point( aoi, fixation->screen_point[0],
                    fixation->screen_point[1] ) )
        {
            if( aoi->analysis == NULL )
            {
                aoi->analysis = gac_aoi_analysis_create();
                aoi->analysis->first_fixation = gac_fixation_copy( fixation );
                aoi->analysis->aoi_visited_before_count =
                    aoic->analysis->aoi_visited_count;
                aoic->analysis->aoi_visited_count++;
            }
            aoi->analysis->fixation_count++;
            aoi->analysis->dwell_time += fixation->duration;
            aoic->analysis->fixation_count++;
        }
    }

    return true;
}

/******************************************************************************/
bool gac_aoi_collection_assign( gac_aoi_collection_t* aoic, gac_aoi_t* aoi )
{
    gac_aoi_t* aoi_heap;

    if( aoic == NULL || aoi == NULL )
    {
        return false;
    }

    aoi_heap = aoi->_me;
    if( aoi_heap == NULL )
    {
        aoi_heap = gac_aoi_copy( aoi );
    }

    aoic->aois.items[aoic->aois.count] = aoi_heap;
    aoic->aois.count++;

    return true;
}

/******************************************************************************/
gac_aoi_collection_t* gac_aoi_collection_create()
{
    gac_aoi_collection_t* aoic = malloc( sizeof( gac_aoi_collection_t ) );

    if( aoic == NULL )
    {
        return NULL;
    }

    if( !gac_aoi_collection_init( aoic ) )
    {
        gac_aoi_collection_destroy( aoic );
        return NULL;
    }

    aoic->_me = aoic;

    return aoic;
}

/******************************************************************************/
void gac_aoi_collection_destroy( gac_aoi_collection_t* aoic )
{
    uint32_t i;

    if( aoic == NULL )
    {
        return;
    }

    for( i = 0; i < aoic->aois.count; i++ )
    {
        if( aoic->aois.items[i] != NULL )
        {
            gac_aoi_destroy( aoic->aois.items[i] );
        }
    }

    if( aoic->_me != NULL )
    {
        free( aoic->_me );
    }
}

/******************************************************************************/
bool gac_aoi_collection_init( gac_aoi_collection_t* aoic )
{
    uint32_t i;

    if( aoic == NULL )
    {
        return false;
    }

    aoic->_me = NULL;
    aoic->analysis = NULL;
    aoic->aois.count = 0;
    for( i = 0; i < GAC_AOI_MAX; i++ )
    {
        aoic->aois.items[i] = NULL;
    }

    return true;
}

/******************************************************************************/
gac_aoi_analysis_t* gac_aoi_analysis_copy( gac_aoi_analysis_t* analysis )
{
    gac_aoi_analysis_t* analysis_copy;

    if( analysis == NULL )
    {
        return NULL;
    }

    analysis_copy = gac_aoi_analysis_create();
    analysis_copy->aoi_visited_before_count = analysis->aoi_visited_before_count;
    analysis_copy->dwell_time = analysis->dwell_time;
    analysis_copy->dwell_time_relative = analysis->dwell_time_relative;
    analysis_copy->first_fixation = gac_fixation_copy( analysis->first_fixation );
    analysis_copy->first_saccade = gac_saccade_copy( analysis->first_saccade );
    analysis_copy->fixation_count = analysis->fixation_count;
    analysis_copy->fixation_count_relative = analysis->fixation_count_relative;

    return analysis_copy;
}

/******************************************************************************/
gac_aoi_analysis_t* gac_aoi_analysis_create()
{
    gac_aoi_analysis_t* analysis = malloc(
            sizeof( gac_aoi_analysis_t ) );

    if( analysis == NULL )
    {
        return NULL;
    }

    if( !gac_aoi_analysis_init( analysis ) )
    {
        gac_aoi_analysis_destroy( analysis );
        return NULL;
    }

    analysis->_me = analysis;

    return analysis;
}

/******************************************************************************/
void gac_aoi_analysis_destroy( gac_aoi_analysis_t* analysis )
{
    if( analysis == NULL )
    {
        return;
    }

    if( analysis->first_fixation != NULL )
    {
        gac_fixation_destroy( analysis->first_fixation );
    }

    if( analysis->first_saccade != NULL )
    {
        gac_saccade_destroy( analysis->first_saccade );
    }

    if( analysis->_me != NULL )
    {
        free( analysis->_me );
    }
}

/******************************************************************************/
bool gac_aoi_analysis_init( gac_aoi_analysis_t* analysis )
{
    if( analysis == NULL )
    {
        return false;
    }

    analysis->_me = NULL;
    analysis->fixation_count = 0;
    analysis->fixation_count_relative = 0;
    analysis->aoi_visited_before_count = 0;
    analysis->dwell_time = 0;
    analysis->dwell_time_relative = 0;
    analysis->first_fixation = NULL;
    analysis->first_saccade = NULL;

    return true;
}

/******************************************************************************/
gac_aoi_collection_analysis_t* gac_aoi_collection_analysis_create()
{
    gac_aoi_collection_analysis_t* analysis = malloc(
            sizeof( gac_aoi_collection_analysis_t ) );

    if( analysis == NULL )
    {
        return NULL;
    }

    if( !gac_aoi_collection_analysis_init( analysis ) )
    {
        gac_aoi_collection_analysis_destroy( analysis );
        return NULL;
    }

    analysis->_me = analysis;

    return analysis;
}

/******************************************************************************/
void gac_aoi_collection_analysis_destroy(
        gac_aoi_collection_analysis_t* analysis )
{
    if( analysis == NULL )
    {
        return;
    }

    if( analysis->_me != NULL )
    {
        free( analysis->_me );
    }
}

/******************************************************************************/
bool gac_aoi_collection_analysis_init( gac_aoi_collection_analysis_t* analysis )
{
    if( analysis == NULL )
    {
        return false;
    }

    analysis->_me = NULL;
    analysis->aoi_visited_count = 0;
    analysis->dwell_time = 0;
    analysis->fixation_count = 0;

    return true;
}
