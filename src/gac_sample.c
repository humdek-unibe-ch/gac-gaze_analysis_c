/**
 * @author  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "gac_sample.h"
#include <stdlib.h>
#include <string.h>

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

    if( sample->_me != NULL )
    {
        free( sample->_me );
    }
}

/******************************************************************************/
double gac_sample_get_label_timestamp( gac_sample_t* sample )
{
    return sample->timestamp - sample->label_onset;
}

/******************************************************************************/
double gac_sample_get_onset( gac_sample_t* sample, double ref )
{
    return sample->timestamp - ref;
}

/******************************************************************************/
double gac_sample_get_trial_timestamp( gac_sample_t* sample )
{
    return sample->timestamp - sample->trial_onset;
}

/******************************************************************************/
bool gac_sample_init( gac_sample_t* sample, vec2* screen_point, vec3* origin,
        vec3* point, double timestamp, uint32_t trial_id, const char* label )
{
    if( sample == NULL || screen_point == NULL || origin == NULL
            || point == NULL )
    {
        return false;
    }

    sample->_me = NULL;
    memset( sample->label, '\0', sizeof( sample->label ) );
    if( label != NULL )
    {
        strncpy( sample->label, label, GAC_SAMPLE_MAX_LABEL_LEN - 1 );
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
