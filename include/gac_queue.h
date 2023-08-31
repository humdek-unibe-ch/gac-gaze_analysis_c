/**
 * A queue structure whcih grows dynamically with added items.
 *
 * @file
 *  gac_queue.h
 * @author
 *  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef GAC_QUEUE_H
#define GAC_QUEUE_H

#include <stdint.h>
#include <stdbool.h>

/** ::gac_queue_s */
typedef struct gac_queue_s gac_queue_t;
/** ::gac_queue_item_s */
typedef struct gac_queue_item_s gac_queue_item_t;

/**
 * A generic queue item.
 */
struct gac_queue_item_s
{
    /** A pointer to the next queue item (towards the head). */
    gac_queue_item_t* next;
    /** A pointer to the previous queue item (towards the tail). */
    gac_queue_item_t* prev;
    /** A pointer to the arbitrary data structure */
    void* data;
};

/**
 * A generic queue structure.
 */
struct gac_queue_s
{
    /** Self-pointer to allocated structure for memory management. */ 
    void* _me;
    /** A pointer to the head of the queue to read from. */
    gac_queue_item_t* tail;
    /** A pointer to the tail to write to */
    gac_queue_item_t* head;
    /** The number of occupied spaces. */
    uint32_t count;
    /** The number of total available spaces */
    uint32_t length;
    /** The handler to remove data items */
    void ( *rm )( void* );
};

/**
 * Remove all data items from the queue. The queue remove handler is used to
 * free the data.
 *
 * @param queue
 *  The queue to clear
 * @return
 *  True on success, false on failure.
 */
bool gac_queue_clear( gac_queue_t* queue );

/**
 * Allocate a new queue structure. This needs to be freed.
 *
 * @param length
 *  The length of the queue.
 * @return
 *  The allocated queue structure.
 */
gac_queue_t* gac_queue_create( uint32_t length );

/**
 * Destroy a queue, all ist items and all data inside the items.
 *
 * @param queue
 *  A pointer to the queue to destroy
 */
void gac_queue_destroy( gac_queue_t* queue );

/**
 * Grow the queue.
 *
 * @param queue
 *  A pointer to the queue to grow.
 * @param count
 *  The number of spaces to add.
 * @return
 *  True on success, false on failure.
 */
bool gac_queue_grow( gac_queue_t* queue, uint32_t count );

/**
 * Initialise a queue structure.
 *
 * @param queue
 *  A pointer to the queue to initialise.
 * @param length
 *  The length of the queue
 * @return
 *  True on success, false on failure.
 */
bool gac_queue_init( gac_queue_t* queue, uint32_t length );

/**
 * Remove a the data from the head of the queue and link the the now free space
 * to the tail of the queue.
 *
 * @param queue
 *  A pointer to the queue.
 * @param data
 *  An optional location to store the popped data.
 * @return
 *  True on success, false on failure.
 */
bool gac_queue_pop( gac_queue_t* queue, void** data );

/**
 * Add a new item to the tail of the queue. If no more space is available, the
 * queue is grown by one.
 *
 * @param queue
 *  A pointer to the queue.
 * @param data
 *  The data sample to be added to the tail of the queue.
 * @return
 *  True on success, false on failure.
 */
bool gac_queue_push( gac_queue_t* queue, void* data );

/**
 * The same as gac_queue_pop() but also freeing the data item with the
 * configured remove handler.
 *
 * @param queue
 *  A pointer to the queue.
 * @return
 *  True on success, false on failure.
 */
bool gac_queue_remove( gac_queue_t* queue );

/**
 * Set a remove handler which will be called whenever an item is removed from
 * the queue.
 *
 * @param queue
 *  A pointer to the queue.
 * @param rm
 *  The renmove handler.
 * @return
 *  True on success, false on failure.
 */
bool gac_queue_set_rm_handler( gac_queue_t* queue, void ( *rm )( void* ));

#endif
