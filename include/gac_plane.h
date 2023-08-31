/**
 * Plane definitions to work with 3d to 2d conversions. A plane is defined
 * through three arbitrary points.
 *
 * @file
 *  gac_plane.h
 * @author
 *  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef GAC_PLANE_H
#define GAC_PLANE_H

#include <cglm/vec2.h>
#include <cglm/vec3.h>
#include <cglm/mat4.h>
#include <stdbool.h>

/** ::gac_plane_s */
typedef struct gac_plane_s gac_plane_t;

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

#endif
