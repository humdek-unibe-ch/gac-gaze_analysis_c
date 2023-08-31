/**
 * @author  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "gac_filter_gap.h"

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
