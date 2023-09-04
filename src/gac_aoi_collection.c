/**
 * @author  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "gac_aoi_collection.h"

/******************************************************************************/
bool gac_aoi_collection_add( gac_aoi_collection_t* aoic, gac_aoi_t* aoi )
{
    if( aoic == NULL || aoi == NULL )
    {
        return false;
    }

    gac_aoi_copy_to( &aoic->aois.items[aoic->aois.count], aoi );
    aoic->aois.count++;

    return true;
}

/******************************************************************************/
bool gac_aoi_collection_analyse_finalise( gac_aoi_collection_t* aoic )
{
    uint32_t i;
    gac_aoi_t* aoi;
    if( aoic == NULL )
    {
        return false;
    }

    for( i = 0; i < aoic->aois.count; i++ )
    {
        aoi = &aoic->aois.items[i];
        aoi->analysis.fixation_count_relative =
            ( double )aoi->analysis.fixation_count /
                ( double )aoic->analysis.fixation_count;
        aoi->analysis.dwell_time_relative =
            aoi->analysis.dwell_time / aoic->analysis.dwell_time;
    }

    return true;
}

/******************************************************************************/
bool gac_aoi_collection_analyse_fixation( gac_aoi_collection_t* aoic,
        gac_fixation_t* fixation )
{
    uint32_t i;
    gac_aoi_t* aoi;
    if( aoic == NULL || fixation == NULL )
    {
        return false;
    }

    aoic->analysis.dwell_time += fixation->duration;
    aoic->analysis.fixation_count++;

    for( i = 0; i < aoic->aois.count; i++ )
    {
        aoi = &aoic->aois.items[i];
        if( gac_aoi_includes_point( aoi, fixation->screen_point[0],
                    fixation->screen_point[1] ) )
        {
            if( aoi->analysis.fixation_count == 0 )
            {
                gac_fixation_copy_to( &aoi->analysis.first_fixation, fixation );
                aoi->analysis.aoi_visited_before_count =
                    aoic->analysis.aoi_visited_count;
                aoic->analysis.aoi_visited_count++;
            }
            aoi->analysis.fixation_count++;
            aoi->analysis.dwell_time += fixation->duration;
        }
    }

    return true;
}

/******************************************************************************/
gac_aoi_collection_t* gac_aoi_collection_create()
{
    gac_aoi_collection_t* aoic = malloc( sizeof( gac_aoi_collection_t ) );

    if( aoic == NULL )
    {
        return NULL;
    }

    if( !gac_aoi_collection_init( aoic ) )
    {
        gac_aoi_collection_destroy( aoic );
        return NULL;
    }

    aoic->_me = aoic;

    return aoic;
}

/******************************************************************************/
void gac_aoi_collection_destroy( gac_aoi_collection_t* aoic )
{
    uint32_t i;

    if( aoic == NULL )
    {
        return;
    }

    gac_aoi_collection_analysis_destroy( &aoic->analysis );

    for( i = 0; i < aoic->aois.count; i++ )
    {
        gac_aoi_destroy( &aoic->aois.items[i] );
    }

    if( aoic->_me != NULL )
    {
        free( aoic->_me );
    }
}

/******************************************************************************/
bool gac_aoi_collection_init( gac_aoi_collection_t* aoic )
{
    uint32_t i;

    if( aoic == NULL )
    {
        return false;
    }

    aoic->_me = NULL;
    gac_aoi_collection_analysis_init( &aoic->analysis );
    aoic->aois.count = 0;
    for( i = 0; i < GAC_AOI_MAX; i++ )
    {
        gac_aoi_init( &aoic->aois.items[i], NULL );
    }

    return true;
}
