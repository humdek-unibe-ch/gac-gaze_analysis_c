/**
 * Gaze analysis gap filter implementation.
 *
 * @file
 *  gac_filter_gap.h
 * @author
 *  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef GAC_FILTER_GAP_H
#define GAC_FILTER_GAP_H

#include "gac_queue.h"
#include "gac_sample.h"
#include <stdbool.h>
#include <stdint.h>

/** ::gac_filter_gap_s */
typedef struct gac_filter_gap_s gac_filter_gap_t;

/**
 * The gap fill-in filter structure.
 */
struct gac_filter_gap_s
{
    /** Self-pointer to allocated structure for memory management. */ 
    void* _me;
    /** A flag indicating whether the filter is active or not */
    bool is_enabled;
    /** The maximal allowed gap length to be filled-in */
    double max_gap_length;
    /** The sample period to compute the number of required fill-in samples */
    double sample_period;
};

/**
 * Fill in gaps between the last sample and the current sample if any.
 * The number of samples to be filled in depends on the sample period.
 * To avoid filling up large gaps the gap filling is limited to a maximal
 * gap length (in milliseconds).
 * The sample passed to the function is added as well.
 *
 * @param filter
 *  The gap filter structure holding the configuration parameters.
 * @param samples
 *  The sample queue to be filled in
 * @param sample
 *  The lastes sample
 * @return
 *  The number of samples added to the sample window.
 */
uint32_t gac_filter_gap( gac_filter_gap_t* filter, gac_queue_t* samples,
        gac_sample_t* sample );

/**
 * Allocate the filter gap structure on the heap. this needs to be freed.
 *
 * @param max_gap_length
 *  The maximal gap length in milliseconds to fil-in. Larger gaps are ignored.
 *  If set to 0 the filter is disabled.
 * @param sample_period
 *  The expected average sample period in milliseconds (1000 / sample_rate).
 * @return
 *  A pointer to the allocated filter gap structure.
 */
gac_filter_gap_t* gac_filter_gap_create( double max_gap_length,
        double sample_period );

/**
 * Destroy the gap filter structure.
 *
 * @param filter
 *  A pointer to the structure to destroy.
 */
void gac_filter_gap_destroy( gac_filter_gap_t* filter );

/**
 * Initialise a filter gap structure.
 *
 * @param filter
 *  A pointer to the struct to be initialised.
 * @param max_gap_length
 *  The maximal gap length in milliseconds to fil-in. Larger gaps are ignored.
 *  If set to 0 the filter is disabled.
 * @param sample_period
 *  The expected average sample period in milliseconds (1000 / sample_rate).
 * @return
 *  True on success, false on failure.
 */
bool gac_filter_gap_init( gac_filter_gap_t* filter, double max_gap_length,
        double sample_period );

#endif
