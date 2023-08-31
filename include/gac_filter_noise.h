/**
 * Gaze analysis noise filter implementation.
 *
 * @file
 *  gac_filter_noise.h
 * @author
 *  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef GAC_FILTER_NOISE_H
#define GAC_FILTER_NOISE_H

#include "gac_sample.h"

/** ::gac_filter_noise_s */
typedef struct gac_filter_noise_s gac_filter_noise_t;

/**
 * The available noise filter types
 */
enum gac_filter_noise_type_e
{
    /** Moving average filtering */
    GAC_FILTER_NOISE_TYPE_AVERAGE,
    /** [not implemented] Moving median filtering */
    GAC_FILTER_NOISE_TYPE_MEDIAN,
};

/** #gac_filter_noise_type_e */
typedef enum gac_filter_noise_type_e gac_filter_noise_type_t;

/**
 * The noise filter parameters.
 */
struct gac_filter_noise_s
{
    /** Self-pointer to allocated structure for memory management. */ 
    void* _me;
    /** A flag indicating whether the noise filter is active or not */
    bool is_enabled;
    /** The noise filter window */
    gac_queue_t window;
    /** The mid-point counter */
    uint32_t mid;
    /** The noise filter type */
    gac_filter_noise_type_t type;
};

/**
 * A noise filter. The filter consecutively collects samples into a window and
 * returns a filtered value when the window is full, otherwise the passed
 * sample is returned. The filter maintains its won sample window.
 *
 * @param filter
 *  The filter parameters.
 * @param sample
 *  The sample to add to the filter window.
 * @return
 *  A filtered sample if the filter window is full or the sample passed to the
 *  function otherwise.
 */
gac_sample_t* gac_filter_noise( gac_filter_noise_t* filter,
        gac_sample_t* sample );

/**
 * Allocate the noise filter structure. This needs to be freed.
 *
 * @param type
 *  The noise filter type.
 * @param mid_idx
 *  The mid index of the window. This is used to compute the length of the
 *  window: window_length = mid_idx * 2 + 1. If set to 0 the filter is disabled.
 * @return
 *  A pointer to the allocated structure or NULL on failure.
 */
gac_filter_noise_t* gac_filter_noise_create( gac_filter_noise_type_t type,
        uint32_t mid_idx );

/**
 * Destroy the noise filter structure.
 *
 * @param filter
 *  A pointer to the structure to destroy.
 */
void gac_filter_noise_destroy( gac_filter_noise_t* filter );

/**
 * Initialises a noise filter structure.
 *
 * @param filter
 *  A pointer to the structure to initialise.
 * @param type
 *  The noise filter type.
 * @param mid_idx
 *  The mid index of the window. This is used to compute the length of the
 *  window: window_length = mid_idx * 2 + 1. If set to 0 the filter is disabled.
 * @return
 *  True on success, false on failure.
 */
bool gac_filter_noise_init( gac_filter_noise_t* filter,
        gac_filter_noise_type_t type, uint32_t mid_idx );

/**
 * A moving average noise filter. It computes the average sample point and
 * origin from all samples in the filter window and assigns the timestamp of
 * the median sample (the sample in the middle of the window) to the averaged
 * sample.
 *
 * @param filter
 *  The filter parameters
 * @return
 *  A new averaged sample if the filter window is full or the sample passed to
 *  the function otherwise.
 */
gac_sample_t* gac_filter_noise_average( gac_filter_noise_t* filter );

#endif
