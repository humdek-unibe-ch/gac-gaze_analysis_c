/**
 * The analysis structure definition of an AOI.
 *
 * @file
 *  gac_aoi_analysis.h
 * @author
 *  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef GAC_AOI_ANALYSIS_H
#define GAC_AOI_ANALYSIS_H

#include <stdint.h>
#include "gac_fixation.h"
#include "gac_saccade.h"


/** ::gac_aoi_analysis_s */
typedef struct gac_aoi_analysis_s gac_aoi_analysis_t;

/**
 * A structure holding the AOI analysis results.
 */
struct gac_aoi_analysis_s
{
    /** Self-pointer to allocated structure for memory management. */ 
    void* _me;
    /**
     * The number of different AOIs visited before the first fixation hit
     * this AOI.
     */
    uint32_t aoi_visited_before_count;
    /**
     * The number of fixations in this AOI.
     */
    uint32_t fixation_count;
    /**
     * The number of saccades entering the AOI.
     */
    uint32_t enter_saccade_count;
    /**
     * The relative number of fixations in this AOI where `1` is the number of
     * all fixations within the trial interest period. The trial interest
     * period corresponds to all samples with the same trial ID.
     */
    double fixation_count_relative;
    /**
     * The sum of all fixation durations on the AOI.
     */
    double dwell_time;
    /**
     * The relative trial time spent on the AOI. `1` is the sum of all fixation
     * durations within the trial interest period. The trial interest period
     * corresponds to all samples with the same trial ID.
     */
    double dwell_time_relative;
    /**
     * The first fixation on the AOI.
     */
    gac_fixation_t first_fixation;
    /**
     * The first saccade on the AOI.
     */
    gac_saccade_t first_saccade;
};

/**
 * Clear an AIO analysis structure.
 *
 * @param analysis
 *  A pointer to the structure to clear.
 * @return
 *  True on success, false on failure.
 */
bool gac_aoi_analysis_clear( gac_aoi_analysis_t* analysis );

/**
 * Create a deep copy of the AOI analysis structure.
 *
 * @param analysis
 *  A pointer to the analysis structure to be copied.
 * @return
 *  A newly allocated copy of the input structure.
 */
gac_aoi_analysis_t* gac_aoi_analysis_copy( gac_aoi_analysis_t* analysis );

/**
 * Copy an AOI analysis structure.
 *
 * @param tgt
 *  A pointer to the analysis structure to copy to.
 * @param tgt
 *  A pointer to the analysis structure to be copied.
 * @return
 *  True on success, false otherwise.
 */
bool gac_aoi_analysis_copy_to( gac_aoi_analysis_t* tgt,
        gac_aoi_analysis_t* src );

/**
 * Allocate a new AOI analysis structure on the heap. This needs to be freed with
 * gac_aoi_analysis_destroy().
 *
 * @return
 *  A pointer to the newly allocated structure.
 */
gac_aoi_analysis_t* gac_aoi_analysis_create();

/**
 * Destroy an AOI analysis structure.
 *
 * @param analysis
 *  A pointer to the analysis structure to destroy.
 */
void gac_aoi_analysis_destroy( gac_aoi_analysis_t* analysis );

/**
 * Initialise an AIO analisis structure.
 *
 * @param analysis
 *  A pointer to the structure to initialise.
 * @return
 *  True on success, false on failure.
 */
bool gac_aoi_analysis_init( gac_aoi_analysis_t* analysis );

#endif
