/**
 * The AOI collection structure and associated functions. This is used to
 * aggregate information during the AOI analysis.
 *
 * @file
 *  gac_aoi_collection.h
 * @author
 *  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef GAC_AOI_COLLECTION_H
#define GAC_AOI_COLLECTION_H

#include "gac_aoi.h"
#include "gac_aoi_collection_analysis.h"
#include <stdint.h>

/** The maximal allowed area of intersts to analyse. */
#define GAC_AOI_MAX 100

/** ::gac_aoi_collection_s */
typedef struct gac_aoi_collection_s gac_aoi_collection_t;

/**
 * A collection of AOIs.
 */
struct gac_aoi_collection_s
{
    /** Self-pointer to allocated structure for memory management. */ 
    void* _me;
    /** The collection of individual AOIs. */
    struct {
        /** The aoi list. */
        gac_aoi_t items[GAC_AOI_MAX];
        /** The number of AOIs in the list. */
        uint32_t count;
    } aois;
    /** The analysis data of the AOI collection. */
    gac_aoi_collection_analysis_t analysis;
};

/**
 * Add an AOI to an AOI collection.
 *
 * @param aoic
 *  A pointer to the AOI collection
 * @param aoi
 *  A pointer to the AOI to add.
 */
bool gac_aoi_collection_add( gac_aoi_collection_t* aoic, gac_aoi_t* aoi );

/**
 * Finalise the AOI analisis. This function computes the relative values in
 * each AOI structure based on the collection analysis data.
 *
 * @param aoic
 *  A pointer to the AOI collection.
 */
bool gac_aoi_collection_analyse_finalise( gac_aoi_collection_t* aoic );

/**
 * Add a fixation point to the AOI collection and updat ethe analysis.
 *
 * @param aoic
 *  A pointer to an AOI collection.
 * @param fixation
 *  The fixation point to add.
 * @return
 *  True on success, false on failure.
 */
bool gac_aoi_collection_analyse_fixation( gac_aoi_collection_t* aoic,
        gac_fixation_t* fixation );

/**
 * Assign a new AOI to an AOI collection. This function acts similar to
 * gac_aoi_collection_add() but only creates a copy if the AOI to assign is
 * allocated on the stack. If a heap allocated AOI is assigned the AOI is not
 * copied and must, therefore, no longer be freed as it will be freed
 * automatically with gac_aoi_collection_destroy().
 *
 * @param aoic
 *  A pointer to an AOI collection.
 * @param aoi
 *  A pointer to the AOI to be assigned.
 * @return
 *  True on success, false otherwise.
 */
bool gac_aoi_collection_assign( gac_aoi_collection_t* aoic, gac_aoi_t* aoi );

/**
 * Allocate a new AOI collection on the heap.
 *
 * @return
 *  A pointer to the newly allocated AOI collection.
 */
gac_aoi_collection_t* gac_aoi_collection_create();

/**
 * Destroy an AOI collection.
 *
 * @param aoic
 *  Destroy an AOI collection.
 */
void gac_aoi_collection_destroy( gac_aoi_collection_t* aoic );

/**
 * Initialise an AOI collection.
 *
 * @param aoic
 *  A pointer to an AOI collection to initialise.
 * @return
 *  True on success, false otherwise.
 */
bool gac_aoi_collection_init( gac_aoi_collection_t* aoic );

#endif