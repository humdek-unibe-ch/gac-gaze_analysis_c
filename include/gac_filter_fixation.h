/**
 * Gaze analysis fixation filter implementation.
 *
 * @file
 *  gac_filter_fixation.h
 * @author
 *  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef GAC_FILTER_FIXATION_H
#define GAC_FILTER_FIXATION_H

#include "gac_queue.h"
#include "gac_fixation.h"
#include <stdbool.h>
#include <stdint.h>

/** ::gac_filter_fixation_s */
typedef struct gac_filter_fixation_s gac_filter_fixation_t;

/**
 * The fixation filter structure holding filter parameters.
 */
struct gac_filter_fixation_s
{
    /** Self-pointer to allocated structure for memory management. */ 
    void* _me;
    /** The pre-computed dispersion threshold at unit distance */
    double normalized_dispersion_threshold;
    /** The duration threashold */
    double duration_threshold;
    /** A flag indicating whether a fixation is ongoing. */
    bool is_collecting;
    /** A pointer to the sample queue */
    gac_queue_t window;
    /** Counter to keep track of new items in the parent queue. */
    uint32_t new_samples;
    /** The fixation duration */
    double duration;
    /** The fixation screen point */
    vec2 screen_point;
    /** The fixation point */
    vec3 point;
};

/**
 * The fixation detection algorithm I-DT.
 *
 * @param filter
 *  The gap filter structure holding the configuration parameters.
 * @param sample
 *  The lastes sample
 * @param fixation
 *  A location where a detected fixation is stored. This is only valid if the
 *  function returns true.
 * @return
 *  True if a fixation was detected, false otherwise.
 */
bool gac_filter_fixation( gac_filter_fixation_t* filter, gac_sample_t* sample,
        gac_fixation_t* fixation );

/**
 * Allocate a new fixation filter structure on the heap. This structure must be
 * freed.
 *
 * @param dispersion_threshold
 *  The dispersion thresholad in degrees.
 * @param duration_threshold
 *  The duration threshold in milliseconds.
 * @return
 *  The allocated fixation filter structure or NULL on failure.
 */
gac_filter_fixation_t* gac_filter_fixation_create(
        float dispersion_threshold, double duration_threshold );

/**
 * Destroy the fixation filter structure.
 *
 * @param filter
 *  A pointer to the structure to destroy.
 */
void gac_filter_fixation_destroy( gac_filter_fixation_t* filter );

/**
 * Initialise a fixation filter structure.
 *
 * @param filter
 *  The filter structure to initialise.
 * @param dispersion_threshold
 *  The dispersion thresholad in degrees.
 * @param duration_threshold
 *  The duration threshold in milliseconds.
 * @return
 *  True on success, false on failure.
 */
bool gac_filter_fixation_init( gac_filter_fixation_t* filter,
        float dispersion_threshold, double duration_threshold );

#endif
