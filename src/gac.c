/**
 * @author  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "gac.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

#ifndef LIB_GAC_VERSION
#define LIB_GAC_VERSION "undefined"
#endif

/******************************************************************************/
gac_t* gac_create( gac_filter_parameter_t* parameter )
{
    gac_t* h = malloc( sizeof( gac_t ) );
    if( !gac_init( h, parameter ) )
    {
        return NULL;
    }
    h->_me = h;

    return h;
}

/******************************************************************************/
void gac_destroy( gac_t* h )
{
    if( h == NULL )
    {
        return;
    }

    gac_queue_destroy( &h->samples );
    gac_filter_fixation_destroy( &h->fixation );
    gac_filter_saccade_destroy( &h->saccade );
    gac_filter_gap_destroy( &h->gap );
    gac_filter_noise_destroy( &h->noise );
    gac_screen_destroy( h->screen );
    gac_sample_destroy( h->last_sample );

    if( h->_me != NULL )
    {
        free( h->_me );
    }
}

/******************************************************************************/
bool gac_init( gac_t* h, gac_filter_parameter_t* parameter )
{
    if( h == NULL )
    {
        return false;
    }

    h->screen = NULL;
    h->_me = NULL;
    h->last_sample = NULL;
    gac_get_filter_parameter_default( &h->parameter );

    if( parameter != NULL )
    {
        h->parameter.fixation.dispersion_threshold =
            parameter->fixation.dispersion_threshold;
        h->parameter.fixation.duration_threshold =
            parameter->fixation.duration_threshold;
        h->parameter.saccade.velocity_threshold =
            parameter->saccade.velocity_threshold;
        h->parameter.noise.mid_idx = parameter->noise.mid_idx;
        h->parameter.noise.type = parameter->noise.type;
        h->parameter.gap.max_gap_length = parameter->gap.max_gap_length;
        h->parameter.gap.sample_period = parameter->gap.sample_period;
    }

    gac_filter_fixation_init( &h->fixation,
            h->parameter.fixation.dispersion_threshold,
            h->parameter.fixation.duration_threshold );
    gac_queue_set_rm_handler( &h->fixation.window, NULL );
    gac_filter_saccade_init( &h->saccade,
            h->parameter.saccade.velocity_threshold );
    gac_queue_set_rm_handler( &h->saccade.window, NULL );
    gac_filter_noise_init( &h->noise, h->parameter.noise.type,
            h->parameter.noise.mid_idx );
    gac_filter_gap_init( &h->gap, h->parameter.gap.max_gap_length,
            h->parameter.gap.sample_period );

    gac_queue_init( &h->samples, 0 );
    gac_queue_set_rm_handler( &h->samples, gac_sample_destroy );

    return true;
}

/******************************************************************************/
bool gac_get_filter_parameter( gac_t* h, gac_filter_parameter_t* parameter )
{
    if( h == NULL || parameter == NULL )
    {
        return false;
    }

    parameter->fixation.dispersion_threshold =
        h->parameter.fixation.dispersion_threshold;
    parameter->fixation.duration_threshold =
        h->parameter.fixation.duration_threshold;
    parameter->saccade.velocity_threshold =
        h->parameter.saccade.velocity_threshold;
    parameter->noise.mid_idx = h->parameter.noise.mid_idx;
    parameter->noise.type = h->parameter.noise.type;
    parameter->gap.max_gap_length = h->parameter.gap.max_gap_length;
    parameter->gap.sample_period = h->parameter.gap.sample_period;

    return true;
}

/******************************************************************************/
bool gac_get_filter_parameter_default( gac_filter_parameter_t* parameter )
{
    if( parameter == NULL )
    {
        return false;
    }

    parameter->fixation.dispersion_threshold = 0.5;
    parameter->fixation.duration_threshold = 100;
    parameter->saccade.velocity_threshold = 20;
    parameter->noise.mid_idx = 1;
    parameter->noise.type = GAC_FILTER_NOISE_TYPE_AVERAGE;
    parameter->gap.max_gap_length = 50;
    parameter->gap.sample_period = 1000.0/60.0;

    return true;
}

/******************************************************************************/
bool gac_set_screen( gac_t* h,
        float top_left_x, float top_left_y, float top_left_z,
        float top_right_x, float top_right_y, float top_right_z,
        float bottom_left_x, float bottom_left_y, float bottom_left_z )
{
    gac_screen_t* screen;
    vec3 top_left = { top_left_x, top_left_y, top_left_z };
    vec3 top_right = { top_right_x, top_right_y, top_right_z };
    vec3 bottom_left = { bottom_left_x, bottom_left_y, bottom_left_z };

    if( h == NULL )
    {
        return false;
    }

    screen = gac_screen_create( &top_left, &top_right, &bottom_left );
    if( screen == NULL )
    {
        return false;
    }

    gac_screen_destroy( h->screen );
    h->screen = screen;

    return true;
}

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

/******************************************************************************/
bool gac_sample_window_cleanup( gac_t* h )
{
    while( h->samples.count > h->fixation.window.count
            && h->samples.count > h->saccade.window.count )
    {
        gac_queue_remove( &h->samples );
    }

    return true;
}

/******************************************************************************/
bool gac_sample_window_fixation_filter( gac_t* h, gac_fixation_t* fixation )
{
    uint32_t i;
    gac_queue_item_t* current;
    gac_sample_t* sample;

    if( fixation == NULL || h == NULL || h->samples.count == 0 )
    {
        return false;
    }

    current = h->samples.tail;
    for( i = 0; i < h->fixation.new_samples - 1; i++ )
    {
        current = current->next;
    }
    sample = current->data;
    h->fixation.new_samples--;

    return gac_filter_fixation( &h->fixation, sample, fixation );
}

/******************************************************************************/
bool gac_sample_window_saccade_filter( gac_t* h, gac_saccade_t* saccade )
{
    uint32_t i;
    gac_queue_item_t* current;
    gac_sample_t* sample;

    if( h == NULL || saccade == NULL || h->samples.count == 0 )
    {
        return false;
    }

    current = h->samples.tail;
    for( i = 0; i < h->saccade.new_samples - 1; i++ )
    {
        current = current->next;
    }
    sample = current->data;
    h->saccade.new_samples--;

    return gac_filter_saccade( &h->saccade, sample, saccade );
}

/******************************************************************************/
uint32_t gac_sample_window_update( gac_t* h, float ox, float oy, float oz,
        float px, float py, float pz, double timestamp, uint32_t trial_id,
        const char* label )
{
    vec2 screen_point;
    vec3 point;
    vec3 origin;

    origin[0] = ox;
    origin[1] = oy;
    origin[2] = oz;
    point[0] = px;
    point[1] = py;
    point[2] = pz;

    if( h->screen != NULL )
    {
        gac_screen_point( h->screen, &point, &screen_point );
    }
    else
    {
        glm_vec2_zero( screen_point );
    }

    return gac_sample_window_update_vec( h, &screen_point, &origin, &point,
            timestamp, trial_id, label );
}

/******************************************************************************/
uint32_t gac_sample_window_update_vec( gac_t* h, vec2* screen_point, vec3* origin,
        vec3* point, double timestamp, uint32_t trial_id, const char* label )
{
    uint32_t count;
    double time_delta;
    gac_sample_t* sample;

    sample = gac_sample_create( screen_point, origin, point, timestamp,
            trial_id, label );

    if( h->last_sample != NULL )
    {
        time_delta = timestamp - h->last_sample->timestamp;
        if( time_delta > 0 )
        {
            if( trial_id == h->last_sample->trial_id )
            {
                sample->trial_onset = h->last_sample->trial_onset + time_delta;
            }

            if( label != NULL && h->last_sample->label != NULL
                    && strcmp( label, h->last_sample->label ) == 0 )
            {
                sample->label_onset = h->last_sample->label_onset + time_delta;
            }
        }
    }

    sample = gac_filter_noise( &h->noise, sample );
    count = gac_filter_gap( &h->gap, &h->samples, sample );
    h->fixation.new_samples = count;
    h->saccade.new_samples = count;
    if( h->samples.tail != NULL )
    {
        gac_sample_destroy( h->last_sample );
        h->last_sample = gac_sample_copy( h->samples.tail->data );
    }

    return count;
}

/******************************************************************************/
uint32_t gac_sample_window_update_screen( gac_t* h, float ox, float oy, float oz,
        float px, float py, float pz, float sx, float sy, double timestamp,
        uint32_t trial_id, const char* label )
{
    vec2 screen_point;
    vec3 point;
    vec3 origin;

    screen_point[0] = sx;
    screen_point[1] = sy;
    origin[0] = ox;
    origin[1] = oy;
    origin[2] = oz;
    point[0] = px;
    point[1] = py;
    point[2] = pz;

    return gac_sample_window_update_vec( h, &screen_point, &origin, &point,
            timestamp, trial_id, label );
}

/******************************************************************************/
const char* gac_version()
{
    return LIB_GAC_VERSION;
}
