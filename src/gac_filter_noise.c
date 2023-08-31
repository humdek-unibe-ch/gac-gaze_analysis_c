/**
 * @author  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "gac_filter_noise.h"

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
