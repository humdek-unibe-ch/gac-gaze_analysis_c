/**
 * Screen definitions to work with 3d to 2d conversions. A screen is defined
 * through three points building the top left, top right and bottom left points
 * of a rectangle. The width of a screen is defined by the length of the vector
 * top left -> top right and the height of the screen is defined by the length
 * of the vector top left -> bottom left.
 *
 * @file
 *  gac_screen.h
 * @author
 *  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef GAC_SCREEN_H
#define GAC_SCREEN_H

#include "gac_plane.h"

/** ::gac_screen_s */
typedef struct gac_screen_s gac_screen_t;

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
    /** The width of the screen resolution. */
    float resolution_x;
    /** The height of the screen resolution. */
    float resolution_y;
    /** The screen origin in 2d space. */
    vec2 origin;
    /** The underlying plane definition of the screen */
    gac_plane_t plane;
};

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
 * The same as gac_screen_point() but storing the resulting 2d point in terms
 * of screen resolution with (0, 0) being the top left corner of the screen.
 * Note that this function will always return false if
 * gac_screen_set_resolution() was never called.
 */
bool gac_screen_point_res( gac_screen_t* screen, vec3* point3d,
        vec2* point2d );

/**
 * Set the screen resolution. This allows to use all functions with an `res`
 * suffix. These functions will act exactly like their counter part function
 * without the `res` suffix but use 2d points expressed in the screen
 * resolution.
 *
 * @param screen
 *  A pointer to a screen structure.
 * @param resolution_x
 *  The width of the screen resolution.
 * @param resolution_y
 *  The height of the screen resolution.
 * @return
 *  True on success, false on failure.
 */
bool gac_screen_set_resolution( gac_screen_t* screen, float resolution_x,
        float resolution_y );

#endif
