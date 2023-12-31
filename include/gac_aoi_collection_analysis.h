/**
 * The analysis structure definition of an AOI collection.
 *
 * @file
 *  gac_aoi_collection_analysis.h
 * @author
 *  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef GAC_AOI_COLLECTION_ANALYSIS_H
#define GAC_AOI_COLLECTION_ANALYSIS_H

#include "gac_aoi.h"
#include <stdint.h>
#include <stdbool.h>

/** ::gac_aoi_collection_analysis_s */
typedef struct gac_aoi_collection_analysis_s gac_aoi_collection_analysis_t;
/** ::gac_aoi_collection_analysis_result_s */
typedef struct gac_aoi_collection_analysis_result_s gac_aoi_collection_analysis_result_t;

/**
 * The AOI collection analysis data structure.
 */
struct gac_aoi_collection_analysis_s
{
    /** Self-pointer to allocated structure for memory management. */ 
    void* _me;
    /** The total fixation count. */
    uint32_t fixation_count;
    /** The number of visited aois. */
    uint32_t aoi_visited_count;
    /** A number distingiushing one trial from another */
    uint32_t trial_id;
    /** The summed duration of all fixations. */
    double dwell_time;
};

/**
 * A collection of AOIs.
 */
struct gac_aoi_collection_analysis_result_s
{
    /** Self-pointer to allocated structure for memory management. */ 
    void* _me;
    /** The collection of individual AOIs. */
    struct {
        /** The aoi analysis list. */
        struct {
            char label[GAC_AOI_MAX_LABEL_LEN];
            gac_aoi_analysis_t analysis;
        } items[GAC_AOI_MAX];
        /** The number of AOIs in the list. */
        uint32_t count;
    } aois;
    /** The trial ID associated to the analysis. */
    uint32_t trial_id;
};

/**
 * Clear the analysis structure.
 *
 * @param analysis
 *  A pointer to the AOI collection analysis structure to clear.
 * @return
 *  True on success, false otherwise.
 */
bool gac_aoi_collection_analysis_clear(
        gac_aoi_collection_analysis_t* analysis );

/**
 * Allocate a new AOI collection analysis structure on the heap.
 *
 * @return
 *  A pointer to the newly allocated structure.
 */
gac_aoi_collection_analysis_t* gac_aoi_collection_analysis_create();

/**
 * Destroy an AOI collection analysis structure.
 *
 * @param analysis
 *  A pointer to the AOI collection analysis structure to be destroied.
 */
void gac_aoi_collection_analysis_destroy(
        gac_aoi_collection_analysis_t* analysis );

/**
 * Initialise an AOI collection analysis structure.
 *
 * @param analysis
 *  A pointer to the AOI collection analysis structure to initialise.
 * @return
 *  True on success, false otherwise.
 */
bool gac_aoi_collection_analysis_init( gac_aoi_collection_analysis_t* analysis );

#endif
