/**
 * @author  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "gac_aoi_collection.h"
#include <string.h>

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
bool gac_aoi_collection_analyse_clear( gac_aoi_collection_t* aoic )
{
    uint32_t i;

    if( aoic == NULL )
    {
        return false;
    }

    gac_aoi_collection_analysis_clear( &aoic->analysis );
    for( i = 0; i < aoic->aois.count; i++ )
    {
        gac_aoi_analysis_clear( &aoic->aois.items[i].analysis );
    }

    return true;
}

/******************************************************************************/
bool gac_aoi_collection_analyse_finalise( gac_aoi_collection_t* aoic,
        gac_aoi_collection_analysis_result_t* analysis )
{
    uint32_t i;
    gac_aoi_t* aoi;
    gac_aoi_analysis_t* res;
    char* label;
    if( aoic == NULL || analysis == NULL )
    {
        return false;
    }

    analysis->aois.count = 0;
    analysis->trial_id = aoic->analysis.trial_id;
    for( i = 0; i < GAC_AOI_MAX; i++ )
    {
        gac_aoi_analysis_init( &analysis->aois.items[i].analysis );
        memset( &analysis->aois.items[i].label, '\0',
                sizeof( analysis->aois.items[i].label ) );
    }

    for( i = 0; i < aoic->aois.count; i++ )
    {
        if( aoic->analysis.fixation_count > 0 )
        {
            aoi = &aoic->aois.items[i];
            aoi->analysis.fixation_count_relative =
                ( double )aoi->analysis.fixation_count /
                    ( double )aoic->analysis.fixation_count;
            aoi->analysis.dwell_time_relative =
                aoi->analysis.dwell_time / aoic->analysis.dwell_time;
            res = &analysis->aois.items[analysis->aois.count].analysis;
            label = analysis->aois.items[analysis->aois.count].label;
            gac_aoi_analysis_copy_to( res, &aoi->analysis );
            strncpy( label, aoi->label, GAC_AOI_MAX_LABEL_LEN - 1 );
            analysis->aois.count++;
        }
    }

    gac_aoi_collection_analysis_clear( &aoic->analysis );
    for( i = 0; i < aoic->aois.count; i++ )
    {
        gac_aoi_analysis_clear( &aoic->aois.items[i].analysis );
    }

    return true;
}

/******************************************************************************/
bool gac_aoi_collection_analyse_fixation( gac_aoi_collection_t* aoic,
        gac_fixation_t* fixation,
        gac_aoi_collection_analysis_result_t* analysis )
{
    bool res = false;
    uint32_t i;
    gac_aoi_t* aoi;
    if( aoic == NULL || fixation == NULL )
    {
        return false;
    }

    if( aoic->analysis.trial_id != fixation->first_sample.trial_id )
    {
        res = gac_aoi_collection_analyse_finalise( aoic, analysis );
    }

    aoic->analysis.trial_id = fixation->first_sample.trial_id;
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

    return res;
}

/******************************************************************************/
bool gac_aoi_collection_analyse_saccade( gac_aoi_collection_t* aoic,
        gac_saccade_t* saccade )
{
    uint32_t i;
    gac_aoi_t* aoi;
    if( aoic == NULL || saccade == NULL )
    {
        return false;
    }

    for( i = 0; i < aoic->aois.count; i++ )
    {
        aoi = &aoic->aois.items[i];
        if( !gac_aoi_includes_point( aoi, saccade->first_sample.screen_point[0],
                    saccade->first_sample.screen_point[1] )
                && gac_aoi_includes_point( aoi,
                    saccade->last_sample.screen_point[0],
                    saccade->last_sample.screen_point[1] ) )
        {
            if( aoi->analysis.enter_saccade_count == 0 )
            {
                gac_saccade_copy_to( &aoi->analysis.first_saccade, saccade );
            }
            aoi->analysis.enter_saccade_count++;
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
