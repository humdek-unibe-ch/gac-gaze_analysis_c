/**
 * The fixation data structure.
 *
 * @file
 *  gac_fixation.h
 * @author
 *  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef GAC_FIXATION_H
#define GAC_FIXATION_H

#include "gac_sample.h"

/** ::gac_fixation_s */
typedef struct gac_fixation_s gac_fixation_t;

/**
 * A fixation sample.
 */
struct gac_fixation_s
{
    /** Self-pointer to allocated structure for memory management. */ 
    void* _me;
    /** The 2d fixation gaze point on the screen. */
    vec2 screen_point;
    /** The fixation gaze point. */
    vec3 point;
    /** The fixation duration in milliseconds. */
    double duration;
    /** The first sample of the fixation. */
    gac_sample_t first_sample;
};

/**
 *
 */
gac_fixation_t* gac_fixation_copy( gac_fixation_t* fixation );

/**
 * Allocate a new fixation structure on the heap. This structure must be
 * freed.
 *
 * @param screen_point
 *  The fixation screen point.
 * @param point
 *  The fixation point.
 * @param duration
 *  The duration of the fixation.
 * @param first_sample
 *  The first sample in the fixation.
 * @return
 *  The allocated fixation structure or NULL on failure.
 */
gac_fixation_t* gac_fixation_create( vec2* screen_point, vec3* point,
        double duration, gac_sample_t* first_sample );

/**
 * Destroy a fixation structure.
 *
 * @param fixation
 *  A pointer to the fixation structure to destroy.
 */
void gac_fixation_destroy( gac_fixation_t* fixation );

/**
 * Initialise a fixation structure.
 *
 * @param fixation
 *  The fixation structure to initialise.
 * @param screen_point
 *  The fixation screen point.
 * @param point
 *  The fixation point.
 * @param duration
 *  The duration of the fixation.
 * @param first_sample
 *  The first sample in the fixation.
 * @return
 *  True on success, false on failure.
 */
bool gac_fixation_init( gac_fixation_t* fixation, vec2* screen_point,
        vec3* point, double duration, gac_sample_t* first_sample );

/**
 * Compute a dispersion threashold assuming a unit distance. To get the actual
 * dispersion threshold multiply this by the distance of the gaze origin to the
 * gaze point.
 *
 * @param angle
 *  The angel in degrees for which the dispersion threshold is computetd.
 *  Usual values range from 0.5 to 1 degree.
 * @return
 *  The normalized dispersion threshold.
 */
float gac_fixation_normalised_dispersion_threshold( float angle );

#endif
