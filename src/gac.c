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
bool gac_add_aoi( gac_t* h, gac_aoi_t* aoi )
{
    if( h == NULL )
    {
        return false;
    }

    return gac_aoi_collection_add( &h->aoic, aoi );
}

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
    gac_aoi_collection_destroy( &h->aoic );

    if( h->_me != NULL )
    {
        free( h->_me );
    }
}

/******************************************************************************/
bool gac_finalise( gac_t* h, gac_aoi_collection_analysis_result_t* analysis )
{
    if( h == NULL )
    {
        return false;
    }
    return gac_aoi_collection_analyse_finalise( &h->aoic, analysis );
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
    h->trial_timestamp = 0;
    h->label_timestamp = 0;
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
    gac_aoi_collection_init( &h->aoic );

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
    gac_sample_t* sample;

    sample = gac_sample_create( screen_point, origin, point, timestamp,
            trial_id, label );

    if( h->last_sample == NULL )
    {
        h->label_timestamp = sample->timestamp;
        h->trial_timestamp = sample->timestamp;
    }
    else
    {
        if( trial_id != h->last_sample->trial_id )
        {
            h->trial_timestamp = sample->timestamp;
        }
        if( label != NULL && strcmp( label, h->last_sample->label ) != 0 )
        {
            h->label_timestamp = sample->timestamp;
        }
    }

    sample->trial_onset =  sample->timestamp - h->trial_timestamp;
    sample->label_onset =  sample->timestamp - h->label_timestamp;
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
