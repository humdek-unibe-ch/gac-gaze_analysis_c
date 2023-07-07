#include "gac.h"
#include <stdlib.h>
#include <math.h>

/******************************************************************************/
gac_t* gac_create( gac_filter_parameter_t* parameter )
{
    gac_t* h = malloc( sizeof( gac_t ) );
    if( !gac_init( h, parameter ) )
    {
        return NULL;
    }
    h->is_heap = true;

    return h;
}

/******************************************************************************/
void gac_destroy( gac_t* h )
{
    if( h == NULL )
    {
        return;
    }

    gac_queue_destroy( &h->samples );
    gac_fixation_filter_destroy( &h->fixation );
    gac_saccade_filter_destroy( &h->saccade );
    gac_filter_gap_destroy( &h->gap );
    gac_filter_noise_destroy( &h->noise );

    if( h->is_heap )
    {
        free( h );
    }
}

/******************************************************************************/
bool gac_init( gac_t* h, gac_filter_parameter_t* parameter )
{
    if( h == NULL )
    {
        return false;
    }

    h->is_heap = false;
    h->parameter.fixation.dispersion_threshold = 0.5;
    h->parameter.fixation.duration_threshold = 100;
    h->parameter.saccade.velocity_threshold = 20;
    h->parameter.noise.mid_idx = 1;
    h->parameter.noise.type = GAC_FILTER_NOISE_TYPE_AVERAGE;
    h->parameter.gap.max_gap_length = 50;
    h->parameter.gap.sample_period = 16.67;
    if( parameter != NULL )
    {
        h->parameter.fixation.dispersion_threshold =
            parameter->fixation.dispersion_threshold;
        h->parameter.fixation.duration_threshold =
            parameter->fixation.duration_threshold;
        h->parameter.saccade.velocity_threshold =
            parameter->saccade.velocity_threshold;
        h->parameter.noise.mid_idx = parameter->noise.mid_idx;
        h->parameter.noise.type = parameter->noise.type;
        h->parameter.gap.max_gap_length = parameter->gap.max_gap_length;
        h->parameter.gap.sample_period = parameter->gap.sample_period;
    }

    gac_fixation_filter_init( &h->fixation,
            h->parameter.fixation.dispersion_threshold,
            h->parameter.fixation.duration_threshold );
    gac_saccade_filter_init( &h->saccade,
            h->parameter.saccade.velocity_threshold );
    gac_filter_noise_init( &h->noise, h->parameter.noise.type,
            h->parameter.noise.mid_idx );
    gac_filter_gap_init( &h->gap, h->parameter.gap.max_gap_length,
            h->parameter.gap.sample_period );

    gac_queue_init( &h->samples, 0 );
    gac_queue_set_rm_handler( &h->samples, gac_sample_destroy );

    return true;
}

/******************************************************************************/
bool gac_get_filter_parameter( gac_t* h, gac_filter_parameter_t* parameter )
{
    if( h == NULL || parameter == NULL )
    {
        return false;
    }

    parameter->fixation.dispersion_threshold =
        h->parameter.fixation.dispersion_threshold;
    parameter->fixation.duration_threshold =
        h->parameter.fixation.duration_threshold;
    parameter->saccade.velocity_threshold =
        h->parameter.saccade.velocity_threshold;
    parameter->noise.mid_idx = h->parameter.noise.mid_idx;
    parameter->noise.type = h->parameter.noise.type;
    parameter->gap.max_gap_length = h->parameter.gap.max_gap_length;
    parameter->gap.sample_period = h->parameter.gap.sample_period;

    return true;
}

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

    if( !filter->is_enabled || sample == NULL )
    {
        return 0;
    }

    if( samples->count == 0 )
    {
        gac_queue_push( samples, sample );
        return 1;
    }

    last_sample = samples->tail->data;
    inter_arrival_time = sample->timestamp - last_sample->timestamp;

    if( inter_arrival_time > filter->sample_period
            && inter_arrival_time <= filter->max_gap_length )
    {
        sample_count = round( inter_arrival_time / filter->sample_period ) - 1;
    }

    for( i = 0; i < sample_count; i++ )
    {
        factor = ( i + 1.0 ) / ( sample_count + 1.0 );
        glm_vec3_lerp( last_sample->origin, sample->origin, factor, origin );
        glm_vec3_lerp( last_sample->point, sample->point, factor, point );
        new_sample = gac_sample_create( &origin, &point,
                last_sample->timestamp + ( i + 1 ) * filter->sample_period );
        gac_queue_push( samples, new_sample );
    }

    gac_queue_push( samples, sample );

    return sample_count + 1;
}

/******************************************************************************/
gac_filter_gap_t* gac_filter_gap_create( double max_gap_length,
        double sample_period )
{
    gac_filter_gap_t* filter = malloc( sizeof( gac_filter_gap_t ) );
    if( !gac_filter_gap_init( filter, max_gap_length, sample_period ) )
    {
        return NULL;
    }
    filter->is_heap = true;

    return filter;
}

/******************************************************************************/
void gac_filter_gap_destroy( gac_filter_gap_t* filter )
{
    if( filter == NULL )
    {
        return;
    }

    if( filter->is_heap )
    {
        free( filter );
    }
}

/******************************************************************************/
bool gac_filter_gap_init( gac_filter_gap_t* filter, double max_gap_length,
        double sample_period )
{
    if( filter == NULL )
    {
        return false;
    }

    filter->is_heap = false;
    filter->is_enabled = max_gap_length == 0 ? false : true;
    filter->max_gap_length = max_gap_length;
    filter->sample_period = sample_period;

    return true;
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
        gac_queue_remove( &filter->window );
    }
    gac_queue_push( &filter->window, sample );

    if( filter->window.count < filter->window.length )
    {
        return gac_sample_create( &sample->origin, &sample->point,
                sample->timestamp );
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
    uint32_t count = 0;
    gac_sample_t* sample_mid = sample_mid;
    gac_queue_item_t* mid;
    vec3 point;
    vec3 origin;

    gac_samples_average_point( &filter->window, &point, 0 );
    gac_samples_average_origin( &filter->window, &origin, 0 );

    mid = filter->window.tail;
    while( mid != NULL && mid->next != NULL && count < filter->mid )
    {
        mid = mid->next;
        count++;
    }
    sample_mid = mid->data;

    return gac_sample_create( &origin, &point, sample_mid->timestamp );
}

/******************************************************************************/
gac_filter_noise_t* gac_filter_noise_create( gac_filter_noise_type_t type,
        uint32_t mid_idx )
{
    gac_filter_noise_t* filter = malloc( sizeof( gac_filter_noise_t ) );
    if( !gac_filter_noise_init( filter, type, mid_idx ) )
    {
        return NULL;
    }
    filter->is_heap = true;

    return filter;
}

/******************************************************************************/
void gac_filter_noise_destroy( gac_filter_noise_t* filter )
{
    if( filter == NULL )
    {
        return;
    }

    gac_queue_destroy( &filter->window );
    if( filter->is_heap )
    {
        free( filter );
    }
}

/******************************************************************************/
bool gac_filter_noise_init( gac_filter_noise_t* filter,
        gac_filter_noise_type_t type, uint32_t mid_idx )
{
    if( filter == NULL )
    {
        return false;
    }

    filter->is_heap = false;
    filter->is_enabled = mid_idx == 0 ? false : true;
    filter->type = type;
    filter->mid = mid_idx;
    gac_queue_init( &filter->window, mid_idx * 2 + 1 );
    gac_queue_set_rm_handler( &filter->window, gac_sample_destroy );

    return true;
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
    fixation->is_heap = true;

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
gac_filter_fixation_t* gac_fixation_filter_create(
        float dispersion_threshold, double duration_threshold )
{
    gac_filter_fixation_t* filter = malloc( sizeof( gac_filter_fixation_t ) );
    if( !gac_fixation_filter_init( filter, dispersion_threshold,
                duration_threshold ) )
    {
        return NULL;
    }
    filter->is_heap = true;

    return filter;
}

/******************************************************************************/
void gac_fixation_filter_destroy( gac_filter_fixation_t* filter )
{
    if( filter == NULL )
    {
        return;
    }

    if( filter->is_heap )
    {
        free( filter );
    }
}

/******************************************************************************/
bool gac_fixation_filter_init( gac_filter_fixation_t* filter,
        float dispersion_threshold,
        double duration_threshold )
{
    if( filter == NULL )
    {
        return false;
    }

    filter->is_heap = false;
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

    fixation->is_heap = false;
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
    queue->is_heap = true;

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

    if( queue->is_heap )
    {
        free( queue );
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

    queue->is_heap = false;
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
    void* data;
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
    if( queue == NULL || rm == NULL )
    {
        return false;
    }

    queue->rm = rm;

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
    saccade->is_heap = true;

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
gac_filter_saccade_t* gac_saccade_filter_create( float velocity_threshold )
{
    gac_filter_saccade_t* filter = malloc( sizeof( gac_filter_saccade_t ) );
    if( !gac_saccade_filter_init( filter, velocity_threshold ) )
    {
        return NULL;
    }
    filter->is_heap = true;

    return filter;
}

/******************************************************************************/
void gac_saccade_filter_destroy( gac_filter_saccade_t* filter )
{
    if( filter == NULL )
    {
        return;
    }

    if( filter->is_heap )
    {
        free( filter );
    }
}

/******************************************************************************/
bool gac_saccade_filter_init( gac_filter_saccade_t* filter,
        float velocity_threshold )
{
    if( filter == NULL )
    {
        return false;
    }

    filter->is_heap = false;
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

    saccade->is_heap = false;
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
    sample->is_heap = true;

    return sample;
}

/******************************************************************************/
void gac_sample_destroy( void* data )
{
    gac_sample_t* sample = data;

    if( sample == NULL )
    {
        return;
    }

    if( sample->is_heap )
    {
        free( sample );
    }
}

/******************************************************************************/
bool gac_sample_init( gac_sample_t* sample, vec3* origin, vec3* point,
        double timestamp )
{
    if( sample == NULL )
    {
        return false;
    }

    sample->is_heap = false;
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
        gac_queue_remove( &h->samples );
    }

    return true;
}

/******************************************************************************/
uint32_t gac_sample_window_update( gac_t* h, float ox, float oy, float oz,
        float px, float py, float pz, double timestamp )
{
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
    return gac_filter_gap( &h->gap, &h->samples, sample );
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
