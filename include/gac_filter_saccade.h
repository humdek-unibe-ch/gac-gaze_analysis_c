/**
 * Gaze analysis saccade filter implementation.
 *
 * @file
 *  gac_filter_saccade.h
 * @author
 *  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef GAC_FILTER_SACCADE_H
#define GAC_FILTER_SACCADE_H

#include "gac_saccade.h"

/** ::gac_filter_saccade_s */
typedef struct gac_filter_saccade_s gac_filter_saccade_t;

/**
 * The saccade filter structure holding filter parameters.
 */
struct gac_filter_saccade_s
{
    /** Self-pointer to allocated structure for memory management. */ 
    void* _me;
    /** The velocity threshold */
    float velocity_threshold;
    /** A flag indicating whether a saccade is ongoing */
    bool is_collecting;
    /** Counter to keep track of new items in the parent queue. */
    uint32_t new_samples;
    /** A pointer to the sample queue */
    gac_queue_t window;
};

/**
 * The saccade detection algorithm I-VT.
 *
 * @param filter
 *  The filter parameters
 * @param sample
 *  The lastes sample
 * @param saccade
 *  A location where a detected saccade is stored. This is only valid if the
 *  function returns true.
 * @return
 *  True if a saccade was detected, false otherwise.
 */
bool gac_filter_saccade( gac_filter_saccade_t* filter, gac_sample_t* sample,
        gac_saccade_t* saccade );

/**
 * Allocate a new saccade filter structure on the heap. This needs to be freed.
 *
 * @param velocity_threshold
 *  The velocity threshold in degrees per second.
 * @return
 *  A pointer to the allocated filter structure or NUll on failure.
 */
gac_filter_saccade_t* gac_filter_saccade_create( float velocity_threshold );

/**
 * Destroy the saccade filter structure.
 *
 * @param filter
 *  A pointer to the structure to destroy.
 */
void gac_filter_saccade_destroy( gac_filter_saccade_t* filter );

/**
 * Initialise a saccade filter structure.
 *
 * @param filter
 *  A pointer to the filter structure to initialise.
 * @param velocity_threshold
 *  The velocity threshold in degrees per second.
 * @return
 *  True on success, false on failure.
 */
bool gac_filter_saccade_init( gac_filter_saccade_t* filter,
        float velocity_threshold );

#endif
