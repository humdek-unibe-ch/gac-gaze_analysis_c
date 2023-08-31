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