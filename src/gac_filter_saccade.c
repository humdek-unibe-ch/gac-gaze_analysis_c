/**
 * @author  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "gac_filter_saccade.h"

/******************************************************************************/
bool gac_filter_saccade( gac_filter_saccade_t* filter, gac_sample_t* sample,
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
