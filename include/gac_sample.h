/**
 * Gaze analysis sample definitions.
 *
 * @file
 *  gac.h
 * @author
 *  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef GAC_SAMPLE_H
#define GAC_SAMPLE_H

#include <cglm/vec2.h>
#include <cglm/vec3.h>
#include <stdint.h>
#include <stdbool.h>

/** ::gac_sample_s */
typedef struct gac_sample_s gac_sample_t;

/**
 * The gaze data sample.
 */
struct gac_sample_s
{
    /** Self-pointer to allocated structure for memory management. */ 
    void* _me;
    /** The ID of a ongoing trial. */
    uint32_t trial_id;
    /** The 2d gaze point on the screen. */
    vec2 screen_point;
    /** The gaze point. */
    vec3 point;
    /** The gaze origin. */
    vec3 origin;
    /** The sample timestamp. */
    double timestamp;
    /** The time in milliseconds since the last change of trial ID. */
    double trial_onset;
    /** The time in milliseconds since the last change of label. */
    double label_onset;
    /** Arbitrary label to annotate the sample. */
    char* label;
};

// SAMPLE //////////////////////////////////////////////////////////////////////

/**
 * Allocate a new sample structure on the heap. This needs to be freed.
 *
 * @param screen_point
 *  The 2d screen gaze point vector.
 * @param origin
 *  The gaze origin vector.
 * @param point
 *  The gaze point vector.
 * @param timestamp
 *  The timestamp of the sample.
 * @param trial_id
 *  The ID of the ongoing trial.
 * @param label
 *  An optional arbitrary label annotating the sample.
 * @return
 *  The allocated sample structure or NULL on failure.
 */
gac_sample_t* gac_sample_create( vec2* screen_point, vec3* origin, vec3* point,
        double timestamp, uint32_t trial_id, const char* label );

/**
 * Create a deep copy of a sample. This needs to be freed with
 * gac_sample_destroy().
 *
 * @param sample
 *  The sample to copy
 * @return
 *  A pointer to the new sample or NULL.
 */
gac_sample_t* gac_sample_copy( gac_sample_t* sample );

/**
 * Deep copy of a sample to a target. This needs to be freed with
 * gac_sample_destroy().
 *
 * @param dest
 *  The location where the sample will be copied to.
 * @param sample
 *  The sample to copy
 * @return
 *  A pointer to the new sample or NULL.
 */
bool gac_sample_copy_to( gac_sample_t* dest, gac_sample_t* sample );

/**
 * Destroy a sample structure.
 *
 * @param sample
 *  A pointer to the structure to be destroyed.
 */
void gac_sample_destroy( void* sample );

/**
 * Initialise a sample structure.
 *
 * @param sample
 *  The sample structure to initialise.
 * @param screen_point
 *  The 2d screen gaze point vector.
 * @param origin
 *  The gaze origin vector.
 * @param point
 *  The gaze point vector.
 * @param timestamp
 *  The timestamp of the sample.
 * @param trial_id
 *  The ID of the ongoing trial.
 * @param label
 *  An optional arbitrary label annotating the sample.
 * @return
 *  True on success, false on failure.
 */
bool gac_sample_init( gac_sample_t* sample, vec2* screen_point, vec3* origin,
        vec3* point, double timestamp, uint32_t trial_id, const char* label );

#endif
