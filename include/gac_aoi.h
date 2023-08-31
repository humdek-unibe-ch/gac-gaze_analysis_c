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

#include <stdint.h>
#include <cglm/vec2.h>
#include <cglm/vec3.h>

/** The maximal allowed points definig an area of interest. */
#define GAC_AOI_MAX_POINTS 100

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

#endif
