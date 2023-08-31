/**
 * @author  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "gac_fixation.h"
#include <stdlib.h>
#include <math.h>

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
