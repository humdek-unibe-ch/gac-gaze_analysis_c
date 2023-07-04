#include "gac.h"
#include <stdlib.h>
#include <math.h>

/******************************************************************************/
uint32_t gac_filter_gap( gac_filter_gap_t* filter, gac_queue_t* samples,
        gac_sample_t* sample )
{
    gac_sample_t* last_sample;
    gac_sample_t* new_sample;
    double inter_arrival_time;
    uint32_t i;
    uint32_t sample_count = 0;
    vec3 point;
    vec3 origin;
    double factor;

    if( !filter->is_enabled || samples->count == 0 || sample == NULL )
    {
        return 0;
    }

    last_sample = samples->tail->data;
    inter_arrival_time = sample->timestamp - last_sample->timestamp;

    if( inter_arrival_time > filter->sample_period
            && inter_arrival_time < filter->max_gap_length )
    {
        sample_count = round( inter_arrival_time / filter->sample_period ) - 1;
    }

    for( i = 0; i < sample_count; i++ )
    {
        factor = ( i + 1.0 ) / ( sample_count + 1.0 );
        glm_vec3_lerp( last_sample->origin, sample->origin, factor, origin );
        glm_vec3_lerp( last_sample->point, sample->point, factor, point );
        new_sample = gac_sample_create( &origin, &point,
                last_sample->timestamp + filter->sample_period );
        gac_queue_push( samples, new_sample );
    }

    return sample_count;
}

/******************************************************************************/
gac_sample_t* gac_filter_noise( gac_filter_noise_t* filter,
        gac_sample_t* sample )
{
    if( filter->is_enabled == false || sample == NULL || filter == NULL )
    {
        return sample;
    }

    if( filter->window.count == filter->window.length )
    {
        gac_queue_pop( &filter->window, NULL, true );
    }
    gac_queue_push( &filter->window, sample );

    if( filter->window.count < filter->window.length )
    {
        return sample;
    }

    switch( filter->type )
    {
        case GAC_FILTER_NOISE_TYPE_AVERAGE:
            return gac_filter_noise_average( filter );
        case GAC_FILTER_NOISE_TYPE_MEDIAN:
            return sample;
    }

    return sample;
}

/******************************************************************************/
gac_sample_t* gac_filter_noise_average( gac_filter_noise_t* filter )
{
    uint32_t i;
    gac_sample_t* sample_mid = sample_mid;
    gac_queue_item_t* mid;
    vec3 point;
    vec3 origin;

    gac_samples_average_point( &filter->window, &point, 0 );
    gac_samples_average_origin( &filter->window, &point, 0 );

    mid = filter->window.tail;
    for( i = 0; i < filter->mid; i++ )
    {
        mid = mid->next;
    }
    sample_mid = mid->data;

    return gac_sample_create( &origin, &point, sample_mid->timestamp );
}

/******************************************************************************/
gac_fixation_t* gac_fixation_create( vec3* point, double timestamp,
        double duration )
{
    gac_fixation_t* fixation = malloc( sizeof( gac_fixation_t ) );
    if( !gac_fixation_init( fixation, point, timestamp, duration ) )
    {
        return NULL;
    }

    return fixation;
}

/******************************************************************************/
bool gac_fixation_filter( gac_t* h, gac_fixation_t* fixation )
{
    double duration;
    float dispersion, dispersion_threshold, distance;
    vec3 origin;
    vec3 point;
    gac_sample_t* sample;
    gac_sample_t* first_sample;
    gac_filter_fixation_t* filter;
    gac_queue_t* window;

    if( fixation == NULL || h == NULL )
    {
        return false;
    }
    filter = &h->fixation;
    window = &filter->window;

    if( window->head == NULL )
    {
        window->head = h->samples.head;
    }
    if( window->tail == NULL )
    {
        window->tail = h->samples.head;
    }
    window->tail = window->tail->prev;
    window->count++;

    sample = window->tail->data;
    first_sample = window->head->data;

    duration = sample->timestamp - first_sample->timestamp;
    if( duration >= filter->duration_threshold )
    {
        gac_samples_dispersion( window, &dispersion, 0 );

        gac_samples_average_origin( window, &origin, 0 );
        gac_samples_average_point( window, &point, 0 );
        distance = glm_vec3_distance( origin, point );
        dispersion_threshold = distance * filter->normalized_dispersion_threshold;

        if( dispersion <= dispersion_threshold)
        {
            if( !filter->is_collecting )
            {
                // fixation start
                filter->is_collecting = true;
            }
            filter->duration = duration;
            glm_vec3_copy( point, filter->point );
        }
        else if( filter->is_collecting )
        {
            // fixation stop
            gac_fixation_init( fixation, &filter->point,
                    first_sample->timestamp, filter->duration );
            filter->is_collecting = false;
            window->count = 0;
            window->head = window->tail;
            return true;
        }
        else
        {
            window->count--;
            window->head = window->head->prev;
        }
    }

    return false;
}

/******************************************************************************/
gac_filter_fixation_t* gac_fixation_filter_create( gac_queue_t* samples,
        float dispersion_threshold, double duration_threshold )
{
    gac_filter_fixation_t* filter = malloc( sizeof( gac_filter_fixation_t ) );
    if( !gac_fixation_filter_init( filter, samples, dispersion_threshold,
                duration_threshold ) )
    {
        return NULL;
    }

    return filter;
}

/******************************************************************************/
bool gac_fixation_filter_init( gac_filter_fixation_t* filter,
        gac_queue_t* samples, float dispersion_threshold,
        double duration_threshold )
{
    if( filter == NULL || samples == NULL )
    {
        return false;
    }

    filter->duration = 0;
    glm_vec3_zero( filter->point );
    filter->is_collecting = false;
    filter->duration_threshold = duration_threshold;
    filter->normalized_dispersion_threshold =
        gac_fixation_normalised_dispersion_threshold( dispersion_threshold );
    gac_queue_init( &filter->window, 0 );

    return true;
}

/******************************************************************************/
bool gac_fixation_init( gac_fixation_t* fixation, vec3* point,
        double timestamp, double duration )
{
    if( fixation == NULL )
    {
        return false;
    }

    fixation->duration = duration;
    glm_vec3_copy( *point, fixation->point );
    fixation->timestamp = timestamp;

    return true;
}

/******************************************************************************/
float gac_fixation_normalised_dispersion_threshold( float angle )
{
    return sqrt( 2 * ( 1 - cos( angle * M_PI / 180 ) ) );
}

/******************************************************************************/
gac_queue_t* gac_queue_create( uint32_t length )
{
    gac_queue_t* queue = malloc( sizeof( gac_queue_t ) );
    if( !gac_queue_init( queue, length ) )
    {
        return NULL;
    }

    return queue;
}

/******************************************************************************/
bool gac_queue_grow( gac_queue_t* queue, uint32_t count )
{
    uint32_t i;
    gac_queue_item_t* item = NULL;
    gac_queue_item_t* last_item;

    if( queue == NULL )
    {
        return false;
    }

    last_item = queue->tail;
    while( last_item != NULL )
    {
        last_item = last_item->prev;
    }

    for( i = 0; i < count; i++ )
    {
        last_item = item;
        item = malloc( sizeof( gac_queue_item_t ) );
        item->data = NULL;
        item->prev = NULL;
        item->next = NULL;
        if( last_item != NULL )
        {
            item->next = last_item;
            last_item->prev = item;
        }
        if( queue->tail == NULL )
        {
            queue->tail = item;
        }
        if( queue->head == NULL )
        {
            queue->head = item;
        }
        queue->length++;
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

    queue->count = 0;
    queue->length = 0;
    queue->head = NULL;
    queue->tail = NULL;

    return gac_queue_grow( queue, length );
}

/******************************************************************************/
bool gac_queue_pop( gac_queue_t* queue, void** data, bool free_data )
{
    if( queue->count == 0 )
    {
        return false;
    }

    if( free_data == true )
    {
        free( queue->head->data );
        queue->head->data = NULL;
    }
    if( data != NULL )
    {
        *data = queue->head->data;
    }

    queue->head->data = NULL;
    queue->head->next = queue->tail;
    queue->tail->prev = queue->head;
    queue->head = queue->head->prev;
    queue->head->prev = NULL;
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
gac_saccade_t* gac_saccade_create( vec3* point_start, vec3* point_dest,
        double timestamp, double duration )
{
    gac_saccade_t* saccade = malloc( sizeof( gac_fixation_t ) );
    if( !gac_saccade_init( saccade, point_start, point_dest, timestamp,
            duration ) )
    {
        return NULL;
    }

    return saccade;
}

/******************************************************************************/
bool gac_saccade_filter( gac_t* h, gac_saccade_t* saccade )
{
    gac_sample_t* s1;
    gac_sample_t* s2;
    vec3 v1;
    vec3 v2;
    double duration;
    float angle;
    float velocity;
    gac_filter_saccade_t* filter;
    gac_queue_t* window;

    if( h == NULL || saccade == NULL )
    {
        return false;
    }
    filter = &h->saccade;
    window = &filter->window;

    if( window->head == NULL )
    {
        window->head = h->samples.head;
    }
    if( window->tail == NULL )
    {
        window->tail = h->samples.head;
    }
    window->tail = window->tail->prev;
    window->count++;

    if( window->count < 2 )
    {
        return false;
    }

    s2 = window->tail->data;
    s1 = window->tail->next->data;
    duration = ( s1->timestamp - s2->timestamp ) * 1000;

    glm_vec3_sub( s1->point, s1->origin, v1 );
    glm_vec3_sub( s2->point, s2->origin, v2 );

    angle = glm_vec3_angle( v1, v2 ) * 180 / M_PI;
    velocity =  angle / duration;

    if( velocity > filter->velocity_threshold )
    {
        if( !filter->is_collecting )
        {
            // saccade start
            filter->is_collecting = true;
        }
        filter->duration = duration;
    }
    else if( filter->is_collecting )
    {
        // saccade stop
        s2 = s1;
        s1 = window->head->data;
        gac_saccade_init( saccade, &s1->point, &s2->point,
                s1->timestamp, filter->duration );
        filter->is_collecting = false;
        window->count = 0;
        window->head = window->tail;
        return true;
    }

    return false;
}

/******************************************************************************/
gac_filter_saccade_t* gac_saccade_filter_create( gac_queue_t* samples,
        float velocity_threshold )
{
    gac_filter_saccade_t* filter = malloc( sizeof( gac_filter_saccade_t ) );
    if( !gac_saccade_filter_init( filter, samples, velocity_threshold ) )
    {
        return NULL;
    }

    return filter;
}

/******************************************************************************/
bool gac_saccade_filter_init( gac_filter_saccade_t* filter, gac_queue_t* samples,
        float velocity_threshold )
{
    if( filter == NULL || samples == NULL )
    {
        return false;
    }

    filter->is_collecting = false;
    filter->velocity_threshold = velocity_threshold;
    filter->duration = 0;
    gac_queue_init( &filter->window, 0 );

    return true;
}

/******************************************************************************/
bool gac_saccade_init( gac_saccade_t* saccade, vec3* point_start,
        vec3* point_dest, double timestamp, double duration )
{
    if( saccade == NULL )
    {
        return false;
    }

    saccade->duration = duration;
    glm_vec3_copy( *point_start, saccade->point_start );
    glm_vec3_copy( *point_dest, saccade->point_dest );
    saccade->timestamp = timestamp;

    return true;
}

/******************************************************************************/
gac_sample_t* gac_sample_create( vec3* origin, vec3* point, double timestamp )
{
    gac_sample_t* sample = malloc( sizeof( gac_sample_t ) );
    if( !gac_sample_init( sample, origin, point, timestamp ) )
    {
        return NULL;
    }

    return sample;
}

/******************************************************************************/
bool gac_sample_init( gac_sample_t* sample, vec3* origin, vec3* point,
        double timestamp )
{
    if( sample == NULL )
    {
        return false;
    }

    glm_vec3_copy( *origin, sample->origin );
    glm_vec3_copy( *point, sample->point );
    sample->timestamp = timestamp;

    return true;
}

/******************************************************************************/
bool gac_sample_window_cleanup( gac_t* h )
{
    while( h->samples.count > h->fixation.window.count
            && h->samples.count > h->saccade.window.count )
    {
        gac_queue_pop( &h->samples, NULL, true );
    }

    return true;
}

/******************************************************************************/
uint32_t gac_sample_window_update( gac_t* h, float ox, float oy, float oz,
        float px, float py, float pz, double timestamp )
{
    uint32_t rc;
    vec3 point;
    vec3 origin;
    gac_sample_t* sample;

    origin[0] = ox;
    origin[1] = oy;
    origin[2] = oz;
    point[0] = px;
    point[1] = py;
    point[2] = pz;
    sample = gac_sample_create( &origin, &point, timestamp );

    sample = gac_filter_noise( &h->noise, sample );
    rc = gac_filter_gap( &h->gap, &h->samples, sample );

    if( gac_queue_push( &h->samples, sample ) )
    {
        rc++;
    }

    return rc;
}

/******************************************************************************/
bool gac_samples_average_point( gac_queue_t* samples, vec3* avg,
        uint32_t count )
{
    uint32_t i = 0;
    gac_sample_t* sample;
    gac_queue_item_t* item = samples->tail;

    if( avg == NULL || samples->count == 0 )
    {
        return false;
    }

    glm_vec3_zero( *avg );

    while( item != NULL )
    {
        sample = item->data;
        glm_vec3_add( *avg, sample->point, *avg );
        item = item->next;

        i++;
        if( i == count )
        {
            return true;
        }
    }

    if( count > 0 && i < count )
    {
        return false;
    }

    glm_vec3_divs( *avg, samples->count, *avg );

    return true;
}

/******************************************************************************/
bool gac_samples_average_origin( gac_queue_t* samples, vec3* avg,
        uint32_t count )
{
    uint32_t i = 0;
    gac_sample_t* sample;
    gac_queue_item_t* item = samples->tail;

    if( avg == NULL || samples->count == 0 )
    {
        return false;
    }

    glm_vec3_zero( *avg );

    while( item != NULL )
    {
        sample = item->data;
        glm_vec3_add( *avg, sample->origin, *avg );
        item = item->next;

        i++;
        if( i == count )
        {
            return true;
        }
    }

    if( count > 0 && i < count )
    {
        return false;
    }

    glm_vec3_divs( *avg, samples->count, *avg );

    return true;
}

/******************************************************************************/
bool gac_samples_dispersion( gac_queue_t* samples, float* dispersion,
        uint32_t count )
{
    uint32_t i = 0;
    gac_sample_t* sample;
    gac_queue_item_t* item = samples->tail;
    bool is_first = true;
    vec3 max;
    vec3 min;

    if( dispersion == NULL )
    {
        return false;
    }

    while( item != NULL )
    {
        sample = item->data;
        if( is_first )
        {
            glm_vec3_copy( sample->point, max );
            glm_vec3_copy( sample->point, min );
            is_first = false;
        }
        else
        {
            if( sample->point[0] > max[0] )
            {
                max[0] = sample->point[0];
            }
            if( sample->point[1] > max[1] )
            {
                max[1] = sample->point[1];
            }
            if( sample->point[2] > max[2] )
            {
                max[2] = sample->point[2];
            }
            if( sample->point[0] < min[0] )
            {
                min[0] = sample->point[0];
            }
            if( sample->point[1] < min[1] )
            {
                min[1] = sample->point[1];
            }
            if( sample->point[2] < min[2] )
            {
                min[2] = sample->point[2];
            }
        }
        item = item->next;

        i++;
        if( i == count )
        {
            return true;
        }
    }

    if( count > 0 && i < count )
    {
        return false;
    }

    *dispersion = max[0] - min[0] + max[1] - min[1] + max[2] - min[2];

    return true;
}
