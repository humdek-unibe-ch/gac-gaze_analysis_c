/**
 * @author  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "gac_aoi_analysis.h"

/******************************************************************************/
bool gac_aoi_analysis_clear( gac_aoi_analysis_t* analysis )
{
    vec2 v2d;
    vec3 v3d;
    gac_sample_t sample;

    if( analysis == NULL )
    {
        return false;
    }

    glm_vec2_zero( v2d );
    glm_vec3_zero( v3d );

    analysis->enter_saccade_count = 0;
    analysis->fixation_count = 0;
    analysis->fixation_count_relative = 0;
    analysis->aoi_visited_before_count = 0;
    analysis->dwell_time = 0;
    analysis->dwell_time_relative = 0;
    gac_sample_init( &sample, &v2d, &v3d, &v3d, 0, 0, NULL );
    gac_fixation_init( &analysis->first_fixation, &v2d, &v3d, 0, &sample );
    gac_saccade_init( &analysis->first_saccade, &sample, &sample );

    return true;
}

/******************************************************************************/
gac_aoi_analysis_t* gac_aoi_analysis_copy( gac_aoi_analysis_t* analysis )
{
    gac_aoi_analysis_t* analysis_copy;

    if( analysis == NULL )
    {
        return NULL;
    }

    analysis_copy = gac_aoi_analysis_create();
    if(analysis_copy == NULL )
    {
        return NULL;
    }

    if( !gac_aoi_analysis_copy_to( analysis_copy, analysis ) )
    {
        gac_aoi_analysis_destroy( analysis_copy );
        return NULL;
    }

    return analysis_copy;
}

/******************************************************************************/
bool gac_aoi_analysis_copy_to( gac_aoi_analysis_t* tgt,
        gac_aoi_analysis_t* src )
{
    bool res = true;

    if( tgt == NULL && src == NULL )
    {
        return false;
    }

    res &= gac_aoi_analysis_init( tgt );
    tgt->aoi_visited_before_count = src->aoi_visited_before_count;
    tgt->dwell_time = src->dwell_time;
    tgt->dwell_time_relative = src->dwell_time_relative;
    gac_fixation_copy_to( &tgt->first_fixation, &src->first_fixation );
    gac_saccade_copy_to( &tgt->first_saccade, &src->first_saccade );
    tgt->fixation_count = src->fixation_count;
    tgt->enter_saccade_count = src->enter_saccade_count;
    tgt->fixation_count_relative = src->fixation_count_relative;

    return res;
}

/******************************************************************************/
gac_aoi_analysis_t* gac_aoi_analysis_create()
{
    gac_aoi_analysis_t* analysis = malloc(
            sizeof( gac_aoi_analysis_t ) );

    if( analysis == NULL )
    {
        return NULL;
    }

    if( !gac_aoi_analysis_init( analysis ) )
    {
        gac_aoi_analysis_destroy( analysis );
        return NULL;
    }

    analysis->_me = analysis;

    return analysis;
}

/******************************************************************************/
void gac_aoi_analysis_destroy( gac_aoi_analysis_t* analysis )
{
    if( analysis == NULL )
    {
        return;
    }

    gac_fixation_destroy( &analysis->first_fixation );
    gac_saccade_destroy( &analysis->first_saccade );

    if( analysis->_me != NULL )
    {
        free( analysis->_me );
    }
}

/******************************************************************************/
bool gac_aoi_analysis_init( gac_aoi_analysis_t* analysis )
{
    if( analysis == NULL )
    {
        return false;
    }

    analysis->_me = NULL;
    return gac_aoi_analysis_clear( analysis );
}
