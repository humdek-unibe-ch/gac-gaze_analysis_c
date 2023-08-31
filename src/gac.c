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
bool gac_filter_fixation( gac_filter_fixation_t* filter, gac_sample_t* sample,
        gac_fixation_t* fixation )
{
    return gac_filter_fixation_step( filter, sample, fixation );
}

/******************************************************************************/
gac_filter_fixation_t* gac_filter_fixation_create(
        float dispersion_threshold, double duration_threshold )
{
    gac_filter_fixation_t* filter = malloc( sizeof( gac_filter_fixation_t ) );
    if( !gac_filter_fixation_init( filter, dispersion_threshold,
                duration_threshold ) )
    {
        return NULL;
    }
    filter->_me = filter;

    return filter;
}

/******************************************************************************/
void gac_filter_fixation_destroy( gac_filter_fixation_t* filter )
{
    if( filter == NULL )
    {
        return;
    }

    gac_queue_destroy( &filter->window );
    if( filter->_me != NULL )
    {
        free( filter->_me );
    }
}

/******************************************************************************/
bool gac_filter_fixation_init( gac_filter_fixation_t* filter,
        float dispersion_threshold,
        double duration_threshold )
{
    if( filter == NULL )
    {
        return false;
    }

    filter->_me = NULL;
    filter->duration = 0;
    filter->new_samples = 0;
    glm_vec2_zero( filter->screen_point );
    glm_vec3_zero( filter->point );
    filter->is_collecting = false;
    filter->duration_threshold = duration_threshold;
    filter->normalized_dispersion_threshold =
        gac_fixation_normalised_dispersion_threshold( dispersion_threshold );
    gac_queue_init( &filter->window, 0 );
    gac_queue_set_rm_handler( &filter->window, gac_sample_destroy );

    return true;
}

/******************************************************************************/
bool gac_filter_fixation_step( gac_filter_fixation_t* filter,
        gac_sample_t* sample, gac_fixation_t* fixation )
{
    double duration;
    float dispersion, dispersion_threshold, distance;
    vec3 origin;
    vec3 point;
    vec2 screen_point;
    gac_sample_t* first_sample;
    gac_queue_t* window;

    glm_vec2_zero( screen_point );
    glm_vec3_zero( point );
    glm_vec3_zero( origin );
    if( fixation == NULL || sample == NULL || filter == NULL )
    {
        return false;
    }
    window = &filter->window;
    gac_queue_push( window, sample );

    first_sample = window->head->data;
    duration = sample->timestamp - first_sample->timestamp;
    if( duration < 0 )
    {
        if( filter->duration >= filter->duration_threshold
                && filter->is_collecting )
        {
            goto fixation_stop;
        }
        else
        {
            filter->is_collecting = false;
            gac_queue_clear( window );
            return false;
        }
    }
    else if( duration >= filter->duration_threshold )
    {
        gac_samples_dispersion( window, &dispersion, 0 );
        gac_samples_average_origin( window, &origin, 0 );
        gac_samples_average_point( window, &point, 0 );
        gac_samples_average_screen_point( window, &screen_point, 0 );
        distance = glm_vec3_distance( origin, point );
        dispersion_threshold = distance * filter->normalized_dispersion_threshold;

        if( dispersion <= dispersion_threshold)
        {
            if( !filter->is_collecting )
            {
                // fixation start
                filter->is_collecting = true;
            }
            filter->duration = duration;
            glm_vec3_copy( point, filter->point );
            glm_vec2_copy( screen_point, filter->screen_point );
        }
        else if( filter->is_collecting )
        {
            goto fixation_stop;
        }
        else
        {
            gac_queue_remove( window );
        }
    }

    return false;

fixation_stop:
    gac_fixation_init( fixation, &filter->screen_point, &filter->point,
            filter->duration, first_sample );
    filter->is_collecting = false;
    gac_queue_clear( window );
    return true;

}

/******************************************************************************/
uint32_t gac_filter_gap( gac_filter_gap_t* filter, gac_queue_t* samples,
        gac_sample_t* sample )
{
    gac_sample_t* last_sample;
    gac_sample_t* new_sample;
    double inter_arrival_time;
    uint32_t i;
    uint32_t sample_count = 0;
    vec2 screen_point;
    vec3 point;
    vec3 origin;
    double factor;
    double delta;

    if( sample == NULL )
    {
        return 0;
    }

    if( !filter->is_enabled || samples->count == 0 )
    {
        gac_queue_push( samples, sample );
        return 1;
    }

    last_sample = samples->tail->data;
    inter_arrival_time = sample->timestamp - last_sample->timestamp;

    if( inter_arrival_time > filter->sample_period
            && inter_arrival_time <= filter->max_gap_length )
    {
        sample_count = round( inter_arrival_time / filter->sample_period ) - 1;
    }

    for( i = 0; i < sample_count; i++ )
    {
        factor = ( i + 1.0 ) / ( sample_count + 1.0 );
        delta = ( i + 1 ) * filter->sample_period;
        glm_vec3_lerp( last_sample->origin, sample->origin, factor, origin );
        glm_vec3_lerp( last_sample->point, sample->point, factor, point );
        glm_vec2_lerp( last_sample->screen_point, sample->screen_point, factor,
                screen_point );
        new_sample = gac_sample_create( &screen_point, &origin, &point,
                last_sample->timestamp + delta,
                sample->trial_id, sample->label );
        new_sample->label_onset = sample->label_onset + delta;
        new_sample->trial_onset = sample->trial_onset + delta;
        gac_queue_push( samples, new_sample );
    }

    gac_queue_push( samples, sample );

    return sample_count + 1;
}

/******************************************************************************/
gac_filter_gap_t* gac_filter_gap_create( double max_gap_length,
        double sample_period )
{
    gac_filter_gap_t* filter = malloc( sizeof( gac_filter_gap_t ) );
    if( !gac_filter_gap_init( filter, max_gap_length, sample_period ) )
    {
        return NULL;
    }
    filter->_me = filter;

    return filter;
}

/******************************************************************************/
void gac_filter_gap_destroy( gac_filter_gap_t* filter )
{
    if( filter == NULL )
    {
        return;
    }

    if( filter->_me != NULL )
    {
        free( filter->_me );
    }
}

/******************************************************************************/
bool gac_filter_gap_init( gac_filter_gap_t* filter, double max_gap_length,
        double sample_period )
{
    if( filter == NULL )
    {
        return false;
    }

    filter->_me = NULL;
    filter->is_enabled = max_gap_length == 0 ? false : true;
    filter->max_gap_length = max_gap_length;
    filter->sample_period = sample_period;

    return true;
}

/******************************************************************************/
gac_sample_t* gac_filter_noise( gac_filter_noise_t* filter,
        gac_sample_t* sample )
{
    if( filter->is_enabled == false || sample == NULL || filter == NULL )
    {
        return sample;
    }

    if( filter->window.count == filter->window.length )
    {
        gac_queue_remove( &filter->window );
    }
    gac_queue_push( &filter->window, sample );

    if( filter->window.count < filter->window.length )
    {
        return NULL;
    }

    switch( filter->type )
    {
        case GAC_FILTER_NOISE_TYPE_AVERAGE:
            return gac_filter_noise_average( filter );
        case GAC_FILTER_NOISE_TYPE_MEDIAN:
            return sample;
    }

    return sample;
}

/******************************************************************************/
gac_sample_t* gac_filter_noise_average( gac_filter_noise_t* filter )
{
    uint32_t count = 0;
    gac_sample_t* sample_mid;
    gac_sample_t* sample_new;
    gac_queue_item_t* mid;
    vec2 screen_point;
    vec3 point;
    vec3 origin;

    gac_samples_average_screen_point( &filter->window, &screen_point, 0 );
    gac_samples_average_point( &filter->window, &point, 0 );
    gac_samples_average_origin( &filter->window, &origin, 0 );

    mid = filter->window.tail;
    while( mid != NULL && mid->next != NULL && count < filter->mid )
    {
        mid = mid->next;
        count++;
    }
    sample_mid = mid->data;

    sample_new = gac_sample_create( &screen_point, &origin, &point,
            sample_mid->timestamp, sample_mid->trial_id, sample_mid->label );
    sample_new->label_onset = sample_mid->label_onset;
    sample_new->trial_onset = sample_mid->trial_onset;

    return sample_new;
}

/******************************************************************************/
gac_filter_noise_t* gac_filter_noise_create( gac_filter_noise_type_t type,
        uint32_t mid_idx )
{
    gac_filter_noise_t* filter = malloc( sizeof( gac_filter_noise_t ) );
    if( !gac_filter_noise_init( filter, type, mid_idx ) )
    {
        return NULL;
    }
    filter->_me = filter;

    return filter;
}

/******************************************************************************/
void gac_filter_noise_destroy( gac_filter_noise_t* filter )
{
    if( filter == NULL )
    {
        return;
    }

    gac_queue_destroy( &filter->window );
    if( filter->_me != NULL )
    {
        free( filter->_me );
    }
}

/******************************************************************************/
bool gac_filter_noise_init( gac_filter_noise_t* filter,
        gac_filter_noise_type_t type, uint32_t mid_idx )
{
    if( filter == NULL )
    {
        return false;
    }

    filter->_me = NULL;
    filter->is_enabled = mid_idx == 0 ? false : true;
    filter->type = type;
    filter->mid = mid_idx;
    gac_queue_init( &filter->window, mid_idx * 2 + 1 );
    gac_queue_set_rm_handler( &filter->window, gac_sample_destroy );

    return true;
}

/******************************************************************************/
bool gac_filter_saccade( gac_filter_saccade_t* filter, gac_sample_t* sample,
        gac_saccade_t* saccade )
{
    return gac_filter_saccade_step( filter, sample, saccade );
}

/******************************************************************************/
gac_filter_saccade_t* gac_filter_saccade_create( float velocity_threshold )
{
    gac_filter_saccade_t* filter = malloc( sizeof( gac_filter_saccade_t ) );
    if( !gac_filter_saccade_init( filter, velocity_threshold ) )
    {
        return NULL;
    }
    filter->_me = filter;

    return filter;
}

/******************************************************************************/
void gac_filter_saccade_destroy( gac_filter_saccade_t* filter )
{
    if( filter == NULL )
    {
        return;
    }

    gac_queue_destroy( &filter->window );
    if( filter->_me != NULL )
    {
        free( filter->_me );
    }
}

/******************************************************************************/
bool gac_filter_saccade_init( gac_filter_saccade_t* filter,
        float velocity_threshold )
{
    if( filter == NULL )
    {
        return false;
    }

    filter->_me = NULL;
    filter->is_collecting = false;
    filter->velocity_threshold = velocity_threshold;
    filter->new_samples = 0;
    gac_queue_init( &filter->window, 0 );
    gac_queue_set_rm_handler( &filter->window, gac_sample_destroy );

    return true;
}

/******************************************************************************/
bool gac_filter_saccade_step( gac_filter_saccade_t* filter, gac_sample_t* sample,
        gac_saccade_t* saccade )
{
    gac_sample_t* s1;
    gac_sample_t* s2;
    vec3 v1;
    vec3 v2;
    double duration;
    float angle;
    float velocity;
    gac_queue_t* window;

    if( filter == NULL || saccade == NULL || sample == NULL )
    {
        return false;
    }
    window = &filter->window;
    gac_queue_push( window, sample );

    if( window->count < 2 )
    {
        return false;
    }

    s2 = window->tail->data;
    s1 = window->tail->next->data;
    duration = s2->timestamp - s1->timestamp;

    glm_vec3_sub( s1->point, s1->origin, v1 );
    glm_vec3_sub( s2->point, s2->origin, v2 );

    angle = glm_vec3_angle( v1, v2 ) * 180 / M_PI;
    velocity =  angle / ( duration / 1000 );

    if( velocity > filter->velocity_threshold )
    {
        if( !filter->is_collecting )
        {
            // saccade start
            filter->is_collecting = true;
        }
    }
    else if( filter->is_collecting )
    {
        // saccade stop
        s2 = s1;
        s1 = window->head->data;
        gac_saccade_init( saccade, s1, s2 );
        filter->is_collecting = false;
        gac_queue_clear( window );
        return true;
    }
    else
    {
        gac_queue_remove( window );
    }

    return false;
}

/******************************************************************************/
gac_fixation_t* gac_fixation_create( vec2* screen_point, vec3* point,
        double duration, gac_sample_t* first_sample )
{
    gac_fixation_t* fixation = malloc( sizeof( gac_fixation_t ) );
    if( !gac_fixation_init( fixation, screen_point, point,
                duration, first_sample ) )
    {
        return NULL;
    }
    fixation->_me = fixation;

    return fixation;
}

/******************************************************************************/
void gac_fixation_destroy( gac_fixation_t* fixation )
{
    if( fixation == NULL )
    {
        return;
    }

    gac_sample_destroy( &fixation->first_sample );

    if( fixation->_me != NULL )
    {
        free( fixation->_me );
    }
}

/******************************************************************************/
bool gac_fixation_init( gac_fixation_t* fixation, vec2* screen_point,
        vec3* point, double duration, gac_sample_t* first_sample )
{
    if( fixation == NULL )
    {
        return false;
    }

    fixation->_me = NULL;
    fixation->duration = duration;
    glm_vec2_copy( *screen_point, fixation->screen_point );
    glm_vec3_copy( *point, fixation->point );
    gac_sample_copy_to( &fixation->first_sample, first_sample );

    return true;
}

/******************************************************************************/
float gac_fixation_normalised_dispersion_threshold( float angle )
{
    return sqrt( 2 * ( 1 - cos( angle * M_PI / 180 ) ) );
}

/******************************************************************************/
gac_plane_t* gac_plane_create( vec3* p1, vec3* p2, vec3* p3 )
{
    gac_plane_t* plane = malloc( sizeof( gac_screen_t ) );

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

/******************************************************************************/
gac_saccade_t* gac_saccade_create( gac_sample_t* first_sample,
        gac_sample_t* last_sample )
{
    gac_saccade_t* saccade = malloc( sizeof( gac_saccade_t ) );
    if( !gac_saccade_init( saccade, first_sample, last_sample ) )
    {
        return NULL;
    }
    saccade->_me = saccade;

    return saccade;
}

/******************************************************************************/
void gac_saccade_destroy( gac_saccade_t* saccade )
{
    if( saccade == NULL )
    {
        return;
    }

    gac_sample_destroy( &saccade->first_sample );
    gac_sample_destroy( &saccade->last_sample );

    if( saccade->_me != NULL )
    {
        free( saccade->_me );
    }
}

/******************************************************************************/
bool gac_saccade_init( gac_saccade_t* saccade, gac_sample_t* first_sample,
        gac_sample_t* last_sample )
{
    if( saccade == NULL )
    {
        return false;
    }

    saccade->_me = NULL;
    gac_sample_copy_to( &saccade->first_sample, first_sample );
    gac_sample_copy_to( &saccade->last_sample, last_sample );

    return true;
}

/******************************************************************************/
gac_sample_t* gac_sample_copy( gac_sample_t* sample )
{
    gac_sample_t* new_sample;

    if( sample == NULL )
    {
        return NULL;
    }

    new_sample = gac_sample_create( &sample->screen_point, &sample->origin,
            &sample->point, sample->timestamp, sample->trial_id, sample->label );

    new_sample->label_onset = sample->label_onset;
    new_sample->trial_onset = sample->trial_onset;

    return new_sample;
}

/******************************************************************************/
bool gac_sample_copy_to( gac_sample_t* dest, gac_sample_t* sample )
{
    bool res;
    if( sample == NULL || dest == NULL )
    {
        return false;
    }

    res = gac_sample_init( dest, &sample->screen_point, &sample->origin,
            &sample->point, sample->timestamp, sample->trial_id, sample->label );

    dest->label_onset = sample->label_onset;
    dest->trial_onset = sample->trial_onset;

    return res;
}

/******************************************************************************/
gac_sample_t* gac_sample_create( vec2* screen_point, vec3* origin, vec3* point,
        double timestamp, uint32_t trial_id, const char* label )
{
    gac_sample_t* sample = malloc( sizeof( gac_sample_t ) );
    if( !gac_sample_init( sample, screen_point, origin, point, timestamp,
                trial_id, label ) )
    {
        return NULL;
    }
    sample->_me = sample;

    return sample;
}

/******************************************************************************/
void gac_sample_destroy( void* data )
{
    gac_sample_t* sample = data;

    if( sample == NULL )
    {
        return;
    }

    if( sample->label != NULL )
    {
        free( sample->label );
    }

    if( sample->_me != NULL )
    {
        free( sample->_me );
    }
}

/******************************************************************************/
bool gac_sample_init( gac_sample_t* sample, vec2* screen_point, vec3* origin,
        vec3* point, double timestamp, uint32_t trial_id, const char* label )
{
    if( sample == NULL )
    {
        return false;
    }

    sample->_me = NULL;
    sample->label = NULL;
    if( label != NULL )
    {
        sample->label = strdup( label );
    }
    glm_vec2_copy( *screen_point, sample->screen_point );
    glm_vec3_copy( *origin, sample->origin );
    glm_vec3_copy( *point, sample->point );
    sample->trial_id = trial_id;
    sample->timestamp = timestamp;
    sample->label_onset = 0;
    sample->trial_onset = 0;

    return true;
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

    return gac_filter_fixation_step( &h->fixation, sample, fixation );
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

    return gac_filter_saccade_step( &h->saccade, sample, saccade );
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
bool gac_samples_average_point( gac_queue_t* samples, vec3* avg,
        uint32_t count )
{
    uint32_t i = 0;
    gac_sample_t* sample;
    gac_queue_item_t* item = samples->tail;

    if( avg == NULL || samples->count == 0 )
    {
        return false;
    }

    glm_vec3_zero( *avg );

    while( item != NULL )
    {
        sample = item->data;
        glm_vec3_add( *avg, sample->point, *avg );
        item = item->next;

        i++;
        if( i == count )
        {
            goto success;
        }
    }

    if( count > 0 && i < count )
    {
        return false;
    }

success:
    glm_vec3_divs( *avg, samples->count, *avg );
    return true;
}

/******************************************************************************/
bool gac_samples_average_origin( gac_queue_t* samples, vec3* avg,
        uint32_t count )
{
    uint32_t i = 0;
    gac_sample_t* sample;
    gac_queue_item_t* item = samples->tail;

    if( avg == NULL || samples->count == 0 )
    {
        return false;
    }

    glm_vec3_zero( *avg );

    while( item != NULL )
    {
        sample = item->data;
        glm_vec3_add( *avg, sample->origin, *avg );
        item = item->next;

        i++;
        if( i == count )
        {
            goto success;
        }
    }

    if( count > 0 && i < count )
    {
        return false;
    }

success:
    glm_vec3_divs( *avg, samples->count, *avg );
    return true;
}

/******************************************************************************/
bool gac_samples_average_screen_point( gac_queue_t* samples, vec2* avg,
        uint32_t count )
{
    uint32_t i = 0;
    gac_sample_t* sample;
    gac_queue_item_t* item = samples->tail;

    if( avg == NULL || samples->count == 0 )
    {
        return false;
    }

    glm_vec2_zero( *avg );

    while( item != NULL )
    {
        sample = item->data;
        glm_vec2_add( *avg, sample->screen_point, *avg );
        item = item->next;

        i++;
        if( i == count )
        {
            goto success;
        }
    }

    if( count > 0 && i < count )
    {
        return false;
    }

success:
    glm_vec2_divs( *avg, samples->count, *avg );
    return true;
}

/******************************************************************************/
bool gac_samples_dispersion( gac_queue_t* samples, float* dispersion,
        uint32_t count )
{
    uint32_t i = 0;
    gac_sample_t* sample;
    gac_queue_item_t* item = samples->tail;
    bool is_first = true;
    vec3 max;
    vec3 min;
    glm_vec3_zero( min );
    glm_vec3_zero( max );

    if( dispersion == NULL )
    {
        return false;
    }

    while( item != NULL )
    {
        sample = item->data;
        if( is_first )
        {
            glm_vec3_copy( sample->point, max );
            glm_vec3_copy( sample->point, min );
            is_first = false;
        }
        else
        {
            if( sample->point[0] > max[0] )
            {
                max[0] = sample->point[0];
            }
            if( sample->point[1] > max[1] )
            {
                max[1] = sample->point[1];
            }
            if( sample->point[2] > max[2] )
            {
                max[2] = sample->point[2];
            }
            if( sample->point[0] < min[0] )
            {
                min[0] = sample->point[0];
            }
            if( sample->point[1] < min[1] )
            {
                min[1] = sample->point[1];
            }
            if( sample->point[2] < min[2] )
            {
                min[2] = sample->point[2];
            }
        }
        item = item->next;

        i++;
        if( i == count )
        {
            goto success;
        }
    }

    if( count > 0 && i < count )
    {
        return false;
    }

success:
    *dispersion = max[0] - min[0] + max[1] - min[1] + max[2] - min[2];
    return true;
}

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

    if( !gac_plane_point( &screen->plane, point3d, &p_offset ) )
    {
        return false;
    }

    glm_vec2_sub( p_offset, screen->origin, p );
    ( *point2d )[0] = p[0] / screen->width;
    ( *point2d )[1] = p[1] / screen->height;

    return true;
}

/******************************************************************************/
const char* gac_version()
{
    return LIB_GAC_VERSION;
}
