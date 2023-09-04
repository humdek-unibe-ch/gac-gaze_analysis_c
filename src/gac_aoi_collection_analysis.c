/**
 * @author  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "gac_aoi_collection_analysis.h"
#include <stdlib.h>

/******************************************************************************/
gac_aoi_collection_analysis_t* gac_aoi_collection_analysis_create()
{
    gac_aoi_collection_analysis_t* analysis = malloc(
            sizeof( gac_aoi_collection_analysis_t ) );

    if( analysis == NULL )
    {
        return NULL;
    }

    if( !gac_aoi_collection_analysis_init( analysis ) )
    {
        gac_aoi_collection_analysis_destroy( analysis );
        return NULL;
    }

    analysis->_me = analysis;

    return analysis;
}

/******************************************************************************/
void gac_aoi_collection_analysis_destroy(
        gac_aoi_collection_analysis_t* analysis )
{
    if( analysis == NULL )
    {
        return;
    }

    if( analysis->_me != NULL )
    {
        free( analysis->_me );
    }
}

/******************************************************************************/
bool gac_aoi_collection_analysis_init( gac_aoi_collection_analysis_t* analysis )
{
    if( analysis == NULL )
    {
        return false;
    }

    analysis->_me = NULL;
    analysis->aoi_visited_count = 0;
    analysis->dwell_time = 0;
    analysis->fixation_count = 0;

    return true;
}
