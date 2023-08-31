/**
 * @author  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "gac_filter_fixation.h"

/******************************************************************************/
bool gac_filter_fixation( gac_filter_fixation_t* filter,
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
