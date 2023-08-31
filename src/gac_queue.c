/**
 * @author  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "gac_queue.h"
#include <stdlib.h>

/******************************************************************************/
bool gac_queue_clear( gac_queue_t* queue )
{
    if( queue == NULL )
    {
        return false;
    }

    if( queue->count == 0 )
    {
        return true;
    }

    while( queue->tail != NULL )
    {
        if( queue->tail->data != NULL && queue->rm != NULL )
        {
            queue->rm( queue->tail->data );
            queue->tail->data = NULL;
        }
        queue->count--;
        queue->tail = queue->tail->next;
    }
    queue->tail = queue->head;

    return true;
}

/******************************************************************************/
gac_queue_t* gac_queue_create( uint32_t length )
{
    gac_queue_t* queue = malloc( sizeof( gac_queue_t ) );
    if( !gac_queue_init( queue, length ) )
    {
        return NULL;
    }
    queue->_me = queue;

    return queue;
}

/******************************************************************************/
void gac_queue_destroy( gac_queue_t* queue )
{
    gac_queue_item_t* head;
    gac_queue_item_t* item;

    if( queue == NULL )
    {
        return;
    }

    if( queue->length > 0 )
    {
        head = queue->head;
        while( head != NULL )
        {
            item = head;
            head = head->prev;
            if( item->data != NULL && queue->rm != NULL )
            {
                queue->rm( item->data );
            }
            free( item );
        }
    }

    if( queue->_me != NULL )
    {
        free( queue->_me );
    }
}

/******************************************************************************/
bool gac_queue_grow( gac_queue_t* queue, uint32_t count )
{
    uint32_t i;
    gac_queue_item_t* item = NULL;
    gac_queue_item_t* last_item = NULL;

    if( queue == NULL )
    {
        return false;
    }

    if( queue->tail != NULL )
    {
        last_item = queue->tail->prev;
    }

    for( i = 0; i < count; i++ )
    {
        item = malloc( sizeof( gac_queue_item_t ) );
        item->data = NULL;
        item->next = NULL;
        item->prev = last_item;
        if( last_item != NULL )
        {
            last_item->next = item;
        }
        last_item = item;
        queue->length++;
    }

    if( queue->tail != NULL )
    {
        queue->tail->prev = item;
        item->next = queue->tail;
    }
    else
    {
        queue->tail = item;
    }

    if( queue->head == NULL )
    {
        queue->head = item;
    }

    return true;
}

/******************************************************************************/
bool gac_queue_init( gac_queue_t* queue, uint32_t length )
{
    if( queue == NULL )
    {
        return false;
    }

    queue->_me = NULL;
    queue->count = 0;
    queue->length = 0;
    queue->head = NULL;
    queue->tail = NULL;
    queue->rm = NULL;

    return gac_queue_grow( queue, length );
}

/******************************************************************************/
bool gac_queue_pop( gac_queue_t* queue, void** data )
{
    gac_queue_item_t* free_item;

    if( queue->count == 0 )
    {
        return false;
    }

    free_item = queue->head;

    if( data != NULL )
    {
        *data = free_item->data;
    }

    free_item->data = NULL;
    if( queue->length == 1 )
    {
        queue->head = free_item;
        queue->tail = free_item;
        free_item->next = NULL;
        free_item->prev = NULL;
    }
    else if( queue->count > 1 )
    {
        queue->head = queue->head->prev;
        free_item->next = queue->tail;
        free_item->prev = queue->tail->prev;
        if( free_item->prev != NULL )
        {
            free_item->prev->next = free_item;
        }
        queue->tail->prev = free_item;
    }
    queue->head->next = NULL;
    queue->count--;

    return true;
}

/******************************************************************************/
bool gac_queue_push( gac_queue_t* queue, void* data )
{
    gac_queue_item_t* item;

    if( data == NULL )
    {
        return false;
    }

    if( queue->count == queue->length )
    {
        gac_queue_grow( queue, 1 );
    }
    if( queue->count == 0 )
    {
        item = queue->tail;
    }
    else
    {
        item = queue->tail->prev;
    }

    item->data = data;
    queue->tail = item;
    queue->count++;

    return true;
}

/******************************************************************************/
bool gac_queue_remove( gac_queue_t* queue )
{
    void* data = NULL;
    gac_queue_pop( queue, &data );

    if( data != NULL && queue->rm != NULL )
    {
        queue->rm( data );
    }

    return true;
}

/******************************************************************************/
bool gac_queue_set_rm_handler( gac_queue_t* queue, void ( *rm )( void* ))
{
    if( queue == NULL )
    {
        return false;
    }

    queue->rm = rm;

    return true;
}
