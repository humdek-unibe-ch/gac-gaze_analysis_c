/**
 * Area of interest (AOI) structure and helper functions.
 *
 * @file
 *  gac_aoi.h
 * @author
 *  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef GAC_AOI_H
#define GAC_AOI_H

#include "gac_aoi_analysis.h"
#include <stdint.h>
#include <cglm/vec2.h>
#include <cglm/vec3.h>

/** The maximal allowed points definig an area of interest. */
#define GAC_AOI_MAX_POINTS 100
/** The maximal label length */
#define GAC_AOI_MAX_LABEL_LEN 100

/** ::gac_aoi_s */
typedef struct gac_aoi_s gac_aoi_t;

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
 * An area of interest (AOI) structure.
 */
struct gac_aoi_s
{
    /** Self-pointer to allocated structure for memory management. */ 
    void* _me;
    /** An arbitary point outside the AOI. */
    vec2 ray_origin;
    /** The average length of an AOI edge. */
    float avg_edge_len;
    /** The width of the screen resolution. */
    float resolution_x;
    /** The height of the screen resolution. */
    float resolution_y;
    /** A label describing the aoi. */
    char label[GAC_AOI_MAX_LABEL_LEN];
    /**
     * The points forming the AOI. At least 3 points are required for a
     * valid AOI.
     */
    struct {
        /** The point list. */
        vec2 items[GAC_AOI_MAX_POINTS];
        /** The number of points defining the AOI. */
        uint32_t count;
    } points;
    /**
     * A axis aligned bounding box to quickly do a coars check if a point is
     * outside the polygon.
     */
    struct {
        float x_min;
        float x_max;
        float y_min;
        float y_max;
    } bounding_box;
    /** The analysis data of the AOI. */
    gac_aoi_analysis_t analysis;
};

/**
 * Add a point the AOE definition. An AOE requires at least 3 points to be
 * valid. In addition to attaching the point to the internal array, this
 * function computes a point which is guaranteed to be outside of the AOI at
 * a reasonable distance from the AOI.
 *
 * @param aoi
 *  A pointer to an AOI structure.
 * @param x
 *  The normalised x coordinate of the AOI point to add.
 * @param y
 *  The normalised y coordinate of the AOI point to add.
 * @return
 *  True on success, false on failure.
 */
bool gac_aoi_add_point( gac_aoi_t* aoi, float x, float y );

/**
 * The same as gac_aoi_add_point() but accepting the input coordinates in
 * pixels instead of normalized values. Note that this function will always
 * return false if gac_aoi_set_resolution() was never called.
 */
bool gac_aoi_add_point_res( gac_aoi_t* aoi, float x_res, float y_res );

/**
 * Add four points describing a rectangle to teh AOI, given the top left point,
 * a width and a height.
 *
 * @param aoi
 *  A pointer to the AOI structure.
 * @param x
 *  The normalized x coordinate of the top left point of the rectangle.
 * @param y
 *  The normalized y coordinate of the top left point of the rectangle.
 * @param width
 *  The normalized width of the recatngle.
 * @param height
 *  The normalized height of the recatngle.
 * @return
 *  True on success, false on failure.
 */
bool gac_aoi_add_rect( gac_aoi_t* aoi, float x, float y, float width,
        float height );

/**
 * The same as gac_aoi_add_rect() but accepting the input coordinates in
 * pixels instead of normalized values. Note that this function will always
 * return false if gac_aoi_set_resolution() was never called.
 */
bool gac_aoi_add_rect_res( gac_aoi_t* aoi, float x, float y, float width,
        float height );

/**
 * Create a deep copy of an AOI.
 *
 * @param aoi
 *  A pointer to the AOI to be copied.
 * @return
 *  A newly allocated copy of the input AOI.
 */
gac_aoi_t* gac_aoi_copy( gac_aoi_t* aoi );

/**
 * Copy an AOI structure.
 *
 * @param tgt
 *  A pointer to an AOI to copy to.
 * @param rct
 *  A pointer to the AOI to be copied.
 * @return
 *  True on success, false otherwise.
 */
bool gac_aoi_copy_to( gac_aoi_t* tgt, gac_aoi_t* src );

/**
 * Allocate a new AOI structure. This must be freed with gac_aoi_destroy().
 *
 * @param label
 *  An arbitary label, describing the AOI.
 * @return
 *  A pointer to the allocated structure or NULL on failure.
 */
gac_aoi_t* gac_aoi_create( const char* label );

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
 * The same as gac_aoi_includes_point() but accepting the input coordinates in
 * pixels instead of normalized values. Note that this function will always
 * return false if gac_aoi_set_resolution() was never called.
 */
bool gac_aoi_includes_point_res( gac_aoi_t* aoi, float x_res, float y_res );

/**
 * Initialise the AOI structure.
 *
 * @param aoi
 *  A pointer to the aoi structure to initialise.
 * @param label
 *  An arbitary label, describing the AOI.
 * @return
 *  True on success, false otherwise.
 */
bool gac_aoi_init( gac_aoi_t* aoi, const char* label );

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
 * @param s1
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

/**
 * Set the screen resolution. This allows to use all functions with an `res`
 * suffix. These functions will act exactly like their counter part function
 * without the `res` suffix but use 2d points expressed in the screen
 * resolution.
 *
 * @param aoi
 *  A pointer to an aoi structure.
 * @param resolution_x
 *  The width of the screen resolution.
 * @param resolution_y
 *  The height of the screen resolution.
 * @return
 *  True on success, false on failure.
 */
bool gac_aoi_set_resolution( gac_aoi_t* aoi, float resolution_x,
        float resolution_y );

#endif
