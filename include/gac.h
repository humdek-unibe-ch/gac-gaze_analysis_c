/**
 * Gaze analysis library for fixation and saccade detection in raw gaze data.
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

#ifndef GAC_H
#define GAC_H

#include "gac_queue.h"
#include <stdint.h>
#include <cglm/vec2.h>
#include <cglm/vec3.h>
#include <cglm/mat4.h>
#include <sys/time.h>

#define GAC_AOI_MAX_POINTS 100

/** ::gac_s */
typedef struct gac_s gac_t;
/** ::gac_aoi_s */
typedef struct gac_aoi_s gac_aoi_t;
/** ::gac_sample_s */
typedef struct gac_sample_s gac_sample_t;
/** ::gac_filter_gap_s */
typedef struct gac_filter_gap_s gac_filter_gap_t;
/** ::gac_filter_fixation_s */
typedef struct gac_filter_fixation_s gac_filter_fixation_t;
/** ::gac_filter_noise_s */
typedef struct gac_filter_noise_s gac_filter_noise_t;
/** ::gac_filter_parameter_s */
typedef struct gac_filter_parameter_s gac_filter_parameter_t;
/** ::gac_filter_saccade_s */
typedef struct gac_filter_saccade_s gac_filter_saccade_t;
/** ::gac_fixation_s */
typedef struct gac_fixation_s gac_fixation_t;
/** ::gac_plane_s */
typedef struct gac_plane_s gac_plane_t;
/** ::gac_saccade_s */
typedef struct gac_saccade_s gac_saccade_t;
/** ::gac_screen_s */
typedef struct gac_screen_s gac_screen_t;

/**
 * The order of point triplets. This is used for checking
 * whetehr a point lies within an AOI.
 */
enum gac_aoi_orientation_e
{
    /** Points are colinear. */
    GAC_AOI_ORIENTATION_COLINEAR,
    /** Points are ordered clockwise. */
    GAC_AOI_ORIENTATION_CLOCKWISE,
    /** Points are ordered counter clockwise. */
    GAC_AOI_ORIENTATION_COUNTER_CLOCKWISE,
};

/** #gac_aoi_orientation_e */
typedef enum gac_aoi_orientation_e gac_aoi_orientation_t;

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
 * An area of interest (AOI) structure.
 */
struct gac_aoi_s
{
    /** Self-pointer to allocated structure for memory management. */ 
    void* _me;
    /**
     * The points forming the AOI. At least 3 points are required for a
     * valid AOI.
     */
    vec2 points[GAC_AOI_MAX_POINTS];
    /** An arbitary point outside the AOI. */
    vec2 ray_origin;
    /** The average length of an AOI edge. */
    float avg_edge_len;
    /** The number of points defining the AOI. */
    uint32_t count;
};

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
 * The filter parameter structure to initialise the gaze analysis handeler.
 */
struct gac_filter_parameter_s
{
    /** Self-pointer to allocated structure for memory management. */ 
    void* _me;
    /** The gap filter parameter */
    struct {
        /**
         * The maximal allowed gap length to be filled-in. Set to zero to
         * disable gap fill-in filter.
         */
        double max_gap_length;
        /** The sample period to compute the number of required fill-in samples */
        double sample_period;
    } gap;
    /** Noise filter parameter */
    struct {
        /** The noise filter type. */
        gac_filter_noise_type_t type;
        /**
         * The mid index of the window. This is used to compute the length of
         * the window: window_length = mid_idx * 2 + 1.
         * Set to zero to disable noise filtering.
         */
        uint32_t mid_idx;
    } noise;
    /** Saccade detection. */
    struct {
        /** The velocity threshold in degrees per seconds. */
        float velocity_threshold;
    } saccade;
    /** Fixation detection. */
    struct {
        /** The duration threshold in milliseconds. */
        double duration_threshold;
        /** The dispersion threshold in degrees. */
        float dispersion_threshold;
    } fixation;
};

/**
 * A genaral plane definition.
 */
struct gac_plane_s
{
    /** Self-pointer to allocated structure for memory management. */ 
    void* _me;
    /** A point on the plane 3d space. */
    vec3 p1;
    /** A point on the plane 3d space. */
    vec3 p2;
    /** A point on the plane 3d space. */
    vec3 p3;
    /** The vector pointing from p1 to p2. */
    vec3 e1;
    /** The vector pointing from p1 to p3. */
    vec3 e2;
    /** The normal of the screen surface. */
    vec3 norm;
    /** Transformation matrix to transform a 3d gaze point to a 2d gaze point. */
    mat4 m;
};

/**
 * Screen definition of the eye tracker.
 */
struct gac_screen_s
{
    /** Self-pointer to allocated structure for memory management. */ 
    void* _me;
    /** The width of the screen. */
    float width;
    /** The height of the screen. */
    float height;
    /** The screen origin in 2d space. */
    vec2 origin;
    /** The underlying plane definition of the screen */
    gac_plane_t plane;
};

/**
 * The gaze analysis handler structure.
 */
struct gac_s
{
    /** Self-pointer to allocated structure for memory management. */ 
    void* _me;
    /** The sample queue */
    gac_queue_t samples;
    /** The fixation filter structure */
    gac_filter_fixation_t fixation;
    /** The gap filter structure */
    gac_filter_gap_t gap;
    /** The saccade filetr structure */
    gac_filter_saccade_t saccade;
    /** The noise filter structure */
    gac_filter_noise_t noise;
    /** The parameters passed during configuration */
    gac_filter_parameter_t parameter;
    /** The screen information. */
    gac_screen_t* screen;
    /**
     * The last sample entered to the window. This remains even if the sample
     * window is cleared.
     */
    gac_sample_t* last_sample;
};

// HANDLER /////////////////////////////////////////////////////////////////////

/**
 * Allocate the gaze analysis structure on the heap. This must be freed.
 * If no parameter structure is provided default values are used.
 * Refer to gac_init() for more information.
 *
 * @param parameter
 *  An optional filter parameter structure.
 * @return
 *  A pointer to the allocated structure or NULL on failure.
 */
gac_t* gac_create( gac_filter_parameter_t* parameter );

/**
 * Destroy the gaze analysis handler.
 *
 * @param h
 *  A pointer to the gaze analysis handler.
 */
void gac_destroy( gac_t* h );

/**
 * Initialise the gaze analysis structure.
 *
 * If no parameter structure is provided the following default values are set:
 *  - fixation.dispersion_threshold = 0.5;
 *  - fixation.duration_threshold = 100;
 *  - saccade.velocity_threshold = 20;
 *  - noise.mid_idx = 1;
 *  - noise.type = GAC_FILTER_NOISE_TYPE_AVERAGE;
 *  - gap.max_gap_length = 50;
 *  - gap.sample_period = 16.67;
 *
 * @param h
 *  A pointer to the gaze analysis structure to initialise.
 * @param parameter
 *  An optional filter parameter structure.
 * @return
 *  True on success, false on failure.
 */
bool gac_init( gac_t* h, gac_filter_parameter_t* parameter );

/**
 * Get the filter parameters.
 *
 * @param h
 *  A pointer to the gaze analysis structure to initialise.
 * @param parameter
 *  A location where the filter parameter values can be stored.
 * @return
 *  True on success, false on failure.
 */
bool gac_get_filter_parameter( gac_t* h, gac_filter_parameter_t* parameter );

/**
 * Get the default filter parameter values.
 *
 * @param parameter
 *  A location where the filter parameter values can be stored.
 * @return
 *  True on success, false on failure.
 */
bool gac_get_filter_parameter_default( gac_filter_parameter_t* parameter );

/**
 * Configure the screen position in 3d space. This allows to compute normalized
 * 2d gaze point coordinates.
 *
 * @param h
 *  A pointer to the gaze analysis handler.
 * @param top_left_x
 *  The x coordinate of the top left screen corner.
 * @param top_left_y
 *  The y coordinate of the top left screen corner.
 * @param top_left_z
 *  The z coordinate of the top left screen corner.
 * @param top_right_x
 *  The x coordinate of the top right screen corner.
 * @param top_right_y
 *  The y coordinate of the top right screen corner.
 * @param top_right_z
 *  The z coordinate of the top right screen corner.
 * @param bottom_left_x
 *  The x coordinate of the bottom left screen corner.
 * @param bottom_left_y
 *  The y coordinate of the bottom left screen corner.
 * @param bottom_left_z
 *  The z coordinate of the bottom left screen corner.
 * @return
 *  True on success, false on failure.
 */
bool gac_set_screen( gac_t* h,
        float top_left_x, float top_left_y, float top_left_z,
        float top_right_x, float top_right_y, float top_right_z,
        float bottom_left_x, float bottom_left_y, float bottom_left_z );

// AOI /////////////////////////////////////////////////////////////////////////

/**
 * Add a point the AOE definition. An AOE requires at least 3 points to be
 * valid. In addition to attaching the point to the internal array, this
 * function computes a point which is guaranteed to be outside of the AOI at
 * a reasonable distance from the AOI.
 *
 * @param aoi
 *  A pointer to an aoi structure.
 * @param x
 *  The normalised x coordinate of the AOI point to add.
 * @param y
 *  The normalised y coordinate of the AOI point to add.
 * @return
 *  True on success, false on failure.
 */
bool gac_aoi_add_point( gac_aoi_t* aoi, float x, float y );

/**
 * Allocate a new AOI structure. This must be freed with gac_aoi_destroy().
 *
 * @return
 *  A pointer to the allocated structure or NULL on failure.
 */
gac_aoi_t* gac_aoi_create();

/**
 * Destroies a AOI structure. This works for structures created with
 * gac_aoi_create() as well as gac_aoi_init().
 *
 * @param aoi
 *  A pointer to a AOI structure to destroy.
 */
void gac_aoi_destroy( gac_aoi_t* aoi );

/**
 * Checks whether a point is inside of an AOI. This function uses the ray
 * casting method where a virtual ray is drawn from an arbitraty point outside
 * the AOI to the point. Then, every intersection with segments of the AOI
 * contour is counted.  If an even number of intersection is detected, the
 * point lies outside of the AOI, otherwise the point lies inside the AOI.
 *
 * @param aoi
 *  A pointer to an AOI structure.
 * @param x
 *  The normalised x coordinate of the point to check.
 * @param y
 *  The normalised y coordinate of the point to check.
 * @return
 *  True if the point is inside the AOI, false otherwise.
 */
bool gac_aoi_includes_point( gac_aoi_t* aoi, float x, float y );

/**
 * Initialise the AOI structure.
 *
 * @param aoi
 *  A pointer to the aoi structure to initialise.
 * @return
 *  True on success, false otherwise.
 */
bool gac_aoi_init( gac_aoi_t* aoi );

/**
 * Checks whether the line segment p1q1 intersects with the line segment p2q2.
 *
 * @param p1
 *  A pointer to the staring point of the first segment.
 * @param q1
 *  A pointer to the end point of the first segment.
 * @param p2
 *  A pointer to the staring point of the second segment.
 * @param q2
 *  A pointer to the end point of the second segment.
 * @return
 *  True if the two segments intersect, false otherwise.
 */
bool gac_aoi_intersect( vec2* p1, vec2* q1, vec2* p2, vec2* q2 );

/**
 * Given three colinear points, this function checks if a point p lies on a
 * segment s1s2.
 *
 * @param p
 *  A pointer to the point to check.
 * @parma s1
 *  The starting point of the segment.
 * @param s2
 *  The end point of the segment.
 * @return
 *  True if the point lies on the segment, false otherwise.
 */
bool gac_aoi_point_on_segment( vec2* p, vec2* s1, vec2* s2 );

/**
 * Given three ordered points p, q, and r, this function detects whether the
 * points are colinear, ordered clockwise or counter clockwise.
 *
 * @param p
 *  A pointer to point p.
 * @param q
 *  A pointer to point q.
 * @param r
 *  A pointer to point r.
 * @return
 *  The orientation of the three points.
 */
gac_aoi_orientation_t gac_aoi_orientation_triplet( vec2* p, vec2* q, vec2* r );

// FILTER //////////////////////////////////////////////////////////////////////

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

/**
 * Internal function to compute the fixation detection algorithm I-DT.
 * Do not use this function. INstead use either the function
 * gac_sample_window_fixation_filter() or gac_filter_fixation().
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
bool gac_filter_fixation_step( gac_filter_fixation_t* filter,
        gac_sample_t* sample, gac_fixation_t* fixation );

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

/**
 * Internal function to compute the I-VT algorithm. Do not use this function.
 * Instead use either the function gac_sample_window_saccade_filter() or
 * gac_filter_saccade().
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
bool gac_filter_saccade_step( gac_filter_saccade_t* filter, gac_sample_t* sample,
        gac_saccade_t* saccade );

// FIXATION ////////////////////////////////////////////////////////////////////

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

// PLANE ///////////////////////////////////////////////////////////////////////

/**
 * Allocate a plane in 3d space. This need to be freed with
 * `gac_plane_destroy()`.
 *
 * @param p1
 *  The 3d coordinates of a point in 3d space.
 * @param p2
 *  The 3d coordinates of a point in 3d space.
 * @param p3
 *  The 3d coordinates of a point in 3d space.
 * @return
 *  A pointer to the allocated plane or NULL on failure.
 */
gac_plane_t* gac_plane_create( vec3* p1, vec3* p2, vec3* p3 );

/**
 * Destroy a plane structure.
 *
 * @param plane
 *  A pointer to the plane structure to destroy.
 */
void gac_plane_destroy( gac_plane_t* plane );

/**
 * Initialise a plane in 3d space.
 *
 * @param plane
 *  A pointer to the plane structure to initialise.
 * @param p1
 *  The 3d coordinates of a point in 3d space.
 * @param p2
 *  The 3d coordinates of a point in 3d space.
 * @param p3
 *  The 3d coordinates of a point in 3d space.
 * @return
 *  True on succes and false on failure.
 */
bool gac_plane_init( gac_plane_t* plane, vec3* p1, vec3* p2, vec3* p3 );

/**
 * Compute the 3d intersection point with a plane.
 *
 * @param plane
 *  A pointer to the plane structure.
 * @param origin
 *  The origin of the gaze.
 * @param dir
 *  The gaze direction.
 * @param intersection
 *  A location to store the intersection point. This is only valid if the
 *  function returns true.
 * @return
 *  True if an intersection was found, false otherwise.
 */
bool gac_plane_intersection( gac_plane_t* plane, vec3* origin, vec3* dir,
        vec3* intersection );

/**
 * Transform a 3d gaze point into a 2d point on a plane. This only works for
 * 3d points which coincide with the plane. To compute an intersection
 * use the function gac_plane_intersection().
 *
 * @param plane
 *  A pointer to the plane structure.
 * @param point3d
 *  The 3d point to transform.
 * @param point2d
 *  A location where the 2d point will be stored. This is only valid if the
 *  function returns true.
 * @return
 *  True on success, false otherwise.
 */
bool gac_plane_point( gac_plane_t* plane, vec3* point3d,
        vec2* point2d );

// SACCADE /////////////////////////////////////////////////////////////////////

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

/**
 * Cleanup the sample window. This removes all sample data from the sample
 * window which is no longer used for the gaze analysis.
 *
 * @param h
 *  A pointer to the gaze analysis handler.
 * @return
 *  True on success, false on failure.
 */
bool gac_sample_window_cleanup( gac_t* h );

/**
 * The fixation detection algorithm I-DT. This acts on the sample window managed
 * by the functions gac_sample_window_update() and gac_sample_window_cleanup().
 *
 * @param h
 *  A pointer to the gaze analysis handler.
 * @param fixation
 *  A location where a detected fixation is stored. This is only valid if the
 *  function returns true.
 * @return
 *  True if a fixation was detected, false otherwise.
 */
bool gac_sample_window_fixation_filter( gac_t* h, gac_fixation_t* fixation );

/**
 * The saccade detection algorithm I-VT. This acts on the sample window managed
 * by the functions gac_sample_window_update() and gac_sample_window_cleanup().
 *
 * @param h
 *  A pointer to the gaze analysis handler.
 * @param saccade
 *  A location where a detected saccade is stored. This is only valid if the
 *  function returns true.
 * @return
 *  True if a saccade was detected, false otherwise.
 */
bool gac_sample_window_saccade_filter( gac_t* h, gac_saccade_t* saccade );

/**
 * Update the sample window with a new sample. If noise filtering is enabled
 * the filtered data is added to the sample window and the raw sample is
 * dismissed. If gap filtering is enabled, sample gaps are filled-in with
 * interpolated data samples.
 *
 * @param h
 *  A pointer to the gaze analysis handler.
 * @param ox
 *  The x coordinate of the gaze origin.
 * @param oy
 *  The y coordinate of the gaze origin.
 * @param oz
 *  The z coordinate of the gaze origin.
 * @param px
 *  The x coordinate of the gaze point.
 * @param py
 *  The y coordinate of the gaze point.
 * @param pz
 *  The z coordinate of the gaze point.
 * @param timestamp
 *  The timestamp of the sample.
 * @param trial_id
 *  The ID of the ongoing trial.
 * @param label
 *  An optional arbitrary label annotating the sample.
 * @return
 *  The number of new samples added to the window.
 */
uint32_t gac_sample_window_update( gac_t* h, float ox, float oy, float oz,
        float px, float py, float pz, double timestamp, uint32_t trial_id,
        const char* label );

/**
 * Update sample window with a new sample.
 *
 * @param h
 *  A pointer to the gaze analysis handler.
 * @param screen_point
 *  The 2d screen gaze point
 * @param origin
 *  The gaze origin.
 * @param point
 *  The gaze point.
 * @param timestamp
 *  The timestamp of the sample.
 * @param trial_id
 *  The ID of the ongoing trial.
 * @param label
 *  An optional arbitrary label annotating the sample.
 * @return
 *  The number of new samples added to the window.
 */
uint32_t gac_sample_window_update_vec( gac_t* h, vec2* screen_point, vec3* origin,
        vec3* point, double timestamp, uint32_t trial_id, const char* label );

/**
 * Update the sample window with a new sample. If noise filtering is enabled
 * the filtered data is added to the sample window and the raw sample is
 * dismissed. If gap filtering is enabled, sample gaps are filled-in with
 * interpolated data samples.
 *
 * @param h
 *  A pointer to the gaze analysis handler.
 * @param ox
 *  The x coordinate of the gaze origin.
 * @param oy
 *  The y coordinate of the gaze origin.
 * @param oz
 *  The z coordinate of the gaze origin.
 * @param px
 *  The x coordinate of the gaze point.
 * @param py
 *  The y coordinate of the gaze point.
 * @param pz
 *  The z coordinate of the gaze point.
 * @param sx
 *  The x coordinate of the screen gaze point.
 * @param sy
 *  The y coordinate of the screen gaze point.
 * @param timestamp
 *  The timestamp of the sample.
 * @param trial_id
 *  The ID of the ongoing trial.
 * @param label
 *  An optional arbitrary label annotating the sample.
 * @return
 *  The number of new samples added to the window.
 */
uint32_t gac_sample_window_update_screen( gac_t* h, float ox, float oy, float oz,
        float px, float py, float pz, float sx, float sy, double timestamp,
        uint32_t trial_id, const char* label );

// SAMPLES /////////////////////////////////////////////////////////////////////

/**
 * Compute the average gaze point of samples in the sample window.
 *
 * @param samples
 *  A pointer to the sample window.
 * @param avg
 *  A location to store the average gaze point. This is only valid if the
 *  function returns true.
 * @param count
 *  The number of samples to perform the computation on, starting by the queue
 *  tail (newest first). If 0 is passed, all samples are included.
 */
bool gac_samples_average_point( gac_queue_t* samples, vec3* avg,
        uint32_t count );

/**
 * Compute the average gaze origin of samples in the sample window.
 *
 * @param samples
 *  A pointer to the sample window.
 * @param avg
 *  A location to store the average gaze origin. This is only valid if the
 *  function returns true.
 * @param count
 *  The number of samples to perform the computation on, starting by the queue
 *  tail (newest first). If 0 is passed, all samples are included.
 */
bool gac_samples_average_origin( gac_queue_t* samples, vec3* avg,
        uint32_t count );

/**
 * Compute the average screen gaze point of samples in the sample window.
 *
 * @param samples
 *  A pointer to the sample window.
 * @param avg
 *  A location to store the average gaze point. This is only valid if the
 *  function returns true.
 * @param count
 *  The number of samples to perform the computation on, starting by the queue
 *  tail (newest first). If 0 is passed, all samples are included.
 */
bool gac_samples_average_screen_point( gac_queue_t* samples, vec2* avg,
        uint32_t count );

/**
 * Compute the gaze point dispersion of samples in the sample window.
 *
 * @param samples
 *  A pointer to the sample window.
 * @param dispersion
 *  A location to store the dispersion value. This is only valid if the
 *  function returns true.
 * @param count
 *  The number of samples to perform the computation on, starting by the queue
 *  tail (newest first). If 0 is passed, all samples are included.
 */
bool gac_samples_dispersion( gac_queue_t* samples, float* dispersion,
        uint32_t count );

// SCREEN //////////////////////////////////////////////////////////////////////

/**
 * Allocate the screen structure. This needs to be freed with
 * `gac_screen_destroy()`. The screen is defined through the top left, the top
 * right and the bottom left point of the screen in 3d space. The width, the
 * height, and the bottom right point of the screen are computed based on these
 * three points.  Make sure to provide points that describe a rectangle for
 * this to make sense.
 *
 * @param top_left
 *  The 3d coordinates of the top left screen point.
 * @param top_right
 *  The 3d coordinates of the top right screen point.
 * @param bottom_left
 *  The 3d coordinates of the bottom left screen point.
 * @return
 *  A pointer to the allocated screen structure or NULL on failure.
 */
gac_screen_t* gac_screen_create( vec3* top_left, vec3* top_right,
        vec3* bottom_left );

/**
 * Destroy a screen structure.
 *
 * @param screen
 *  A pointer to the screen structure to destroy
 */
void gac_screen_destroy( gac_screen_t* screen );

/**
 * Initialise a screen structure through the top left, the top right and the
 * bottom left point of the screen in 3d space. The width, the height, and the
 * bottom right point of the screen are computed based on these three points.
 * Make sure to provide points that describe a rectangle for this to make sense.
 *
 * @param screen
 *  A pointer to the screen structure to initialise.
 * @param top_left
 *  The 3d coordinates of the top left screen point.
 * @param top_right
 *  The 3d coordinates of the top right screen point.
 * @param bottom_left
 *  The 3d coordinates of the bottom left screen point.
 * @return
 *  True on succes and false on failure.
 */
bool gac_screen_init( gac_screen_t* screen, vec3* top_left, vec3* top_right,
        vec3* bottom_left );

/**
 * Transform a 3d gaze point into a normalized 2d point on the screen.
 * (0, 0) represents the top left corner of the screen and (1, 1) represents
 * the bottom right corner.
 *
 * @param screen
 *  A pointer to the screen structure.
 * @param point3d
 *  The 3d point to transform.
 * @param point2d
 *  A location where the 2d point will be stored. This is only valid if the
 *  function returns true.
 * @return
 *  True on success, false otherwise.
 */
bool gac_screen_point( gac_screen_t* screen, vec3* point3d,
        vec2* point2d );

/**
 * Returns the version of the library.
 *
 * @return
 *  A version number string of the form `<major>.<minor>.<revision>`.
 */
const char* gac_version();

#endif
