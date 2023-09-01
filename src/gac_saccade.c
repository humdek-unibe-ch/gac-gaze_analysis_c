/**
 * @author  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "gac_saccade.h"
#include <stdlib.h>

/******************************************************************************/
gac_saccade_t* gac_saccade_copy( gac_saccade_t* saccade )
{
    if( saccade == NULL )
    {
        return NULL;
    }

    return gac_saccade_create( &saccade->first_sample, &saccade->last_sample );
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
