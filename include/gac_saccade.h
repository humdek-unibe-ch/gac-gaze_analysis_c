/**
 * The saccade data structure.
 *
 * @file
 *  gac_saccade.h
 * @author
 *  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef GAC_SACCADE_H
#define GAC_SACCADE_H

#include "gac_sample.h"

/** ::gac_saccade_s */
typedef struct gac_saccade_s gac_saccade_t;

/**
 * A saccade sample.
 */
struct gac_saccade_s
{
    /** Self-pointer to allocated structure for memory management. */ 
    void* _me;
    /** The first sample of the saccade. */
    gac_sample_t first_sample;
    /** The last sample of the saccade. */
    gac_sample_t last_sample;
};

/**
 *
 */
gac_saccade_t* gac_saccade_copy( gac_saccade_t* saccade );

/**
 * Allocate a new saccade structure on the heap. This needs to be freed.
 *
 * @param first_sample
 *  The first sample of the saccade, holding the source point.
 * @param last_sample
 *  The last sample of the saccade, holding the target point.
 * @return
 *  The allocated saccade structure on success or NULL on failure.
 */
gac_saccade_t* gac_saccade_create( gac_sample_t* first_sample,
        gac_sample_t* last_sample );

/**
 * Destroy a saccade structure.
 *
 * @param saccade
 *  A pointer to the saccade structure to destroy.
 */
void gac_saccade_destroy( gac_saccade_t* saccade );

/**
 * Initialise a saccade structure.
 *
 * @param saccade
 *  A pointer to the saccade structure to initialise.
 * @param first_sample
 *  The first sample of the saccade, holding the source point.
 * @param last_sample
 *  The last sample of the saccade, holding the target point.
 * @return
 *  True on success, false on failure.
 */
bool gac_saccade_init( gac_saccade_t* saccade, gac_sample_t* first_sample,
        gac_sample_t* last_sample );

#endif
