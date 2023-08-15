#include "gac.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

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
    gac_filter_fixation_destroy( &h->fixation );
    gac_filter_saccade_destroy( &h->saccade );
    gac_filter_gap_destroy( &h->gap );
    gac_filter_noise_destroy( &h->noise );
    gac_screen_destroy( h->screen );

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

    h->screen = NULL;
    h->is_heap = false;
    h->is_normalized = true;
    gac_get_filter_parameter_default( &h->parameter );

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

    gac_filter_fixation_init( &h->fixation,
            h->parameter.fixation.dispersion_threshold,
            h->parameter.fixation.duration_threshold );
    gac_queue_set_rm_handler( &h->fixation.window, NULL );
    gac_filter_saccade_init( &h->saccade,
            h->parameter.saccade.velocity_threshold );
    gac_queue_set_rm_handler( &h->saccade.window, NULL );
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
bool gac_get_filter_parameter_default( gac_filter_parameter_t* parameter )
{
    if( parameter == NULL )
    {
        return false;
    }

    parameter->fixation.dispersion_threshold = 0.5;
    parameter->fixation.duration_threshold = 100;
    parameter->saccade.velocity_threshold = 20;
    parameter->noise.mid_idx = 1;
    parameter->noise.type = GAC_FILTER_NOISE_TYPE_AVERAGE;
    parameter->gap.max_gap_length = 50;
    parameter->gap.sample_period = 1000.0/60.0;

    return true;
}

/******************************************************************************/
bool gac_set_screen( gac_t* h, bool is_normalized,
        float top_left_x, float top_left_y, float top_left_z,
        float top_right_x, float top_right_y, float top_right_z,
        float bottom_left_x, float bottom_left_y, float bottom_left_z,
        float bottom_right_x, float bottom_right_y, float bottom_right_z )
{
    gac_screen_t* screen;
    vec3 top_left = { top_left_x, top_left_y, top_left_z };
    vec3 top_right = { top_right_x, top_right_y, top_right_z };
    vec3 bottom_left = { bottom_left_x, bottom_left_y, bottom_left_z };
    vec3 bottom_right = { bottom_right_x, bottom_right_y, bottom_right_z };

    if( h == NULL )
    {
        return false;
    }

    screen = gac_screen_create( &top_left, &top_right, &bottom_left,
            &bottom_right );
    if( screen == NULL )
    {
        return false;
    }

    h->is_normalized = is_normalized;
    h->screen = screen;

    return true;
}

/******************************************************************************/
bool gac_filter_fixation( gac_filter_fixation_t* filter, gac_sample_t* sample,
        gac_fixation_t* fixation )
{
    bool res;
    gac_filter_step_action_t action;

    gac_queue_push( &filter->window, sample );
    res = gac_filter_fixation_step( filter, sample, fixation, &action );

    switch( action )
    {
        case GAC_FILTER_STEP_ACTION_SHRINK:
            gac_queue_remove( &filter->window );
            break;
        case GAC_FILTER_STEP_ACTION_CLEAR:
            gac_queue_clear( &filter->window );
            break;
        default:
            break;
    }

    return res;
}

/******************************************************************************/
gac_filter_fixation_t* gac_filter_fixation_create(
        float dispersion_threshold, double duration_threshold )
{
    gac_filter_fixation_t* filter = malloc( sizeof( gac_filter_fixation_t ) );
    if( !gac_filter_fixation_init( filter, dispersion_threshold,
                duration_threshold ) )
    {
        return NULL;
    }
    filter->is_heap = true;

    return filter;
}

/******************************************************************************/
void gac_filter_fixation_destroy( gac_filter_fixation_t* filter )
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
bool gac_filter_fixation_init( gac_filter_fixation_t* filter,
        float dispersion_threshold,
        double duration_threshold )
{
    if( filter == NULL )
    {
        return false;
    }

    filter->is_heap = false;
    filter->duration = 0;
    glm_vec2_zero( filter->screen_point );
    glm_vec3_zero( filter->point );
    filter->is_collecting = false;
    filter->duration_threshold = duration_threshold;
    filter->normalized_dispersion_threshold =
        gac_fixation_normalised_dispersion_threshold( dispersion_threshold );
    gac_queue_init( &filter->window, 0 );
    gac_queue_set_rm_handler( &filter->window, gac_sample_destroy );

    return true;
}

/******************************************************************************/
bool gac_filter_fixation_step( gac_filter_fixation_t* filter,
        gac_sample_t* sample, gac_fixation_t* fixation,
        gac_filter_step_action_t* action )
{
    double duration;
    float dispersion, dispersion_threshold, distance;
    vec3 origin;
    vec3 point;
    vec2 screen_point;
    gac_sample_t* first_sample;
    gac_queue_t* window;

    if( action != NULL )
    {
        *action = GAC_FILTER_STEP_ACTION_NONE;
    }

    if( fixation == NULL || sample == NULL || filter == NULL )
    {
        return false;
    }
    window = &filter->window;

    first_sample = window->head->data;
    duration = sample->timestamp - first_sample->timestamp;
    if( duration >= filter->duration_threshold )
    {
        gac_samples_dispersion( window, &dispersion, 0 );
        gac_samples_average_origin( window, &origin, 0 );
        gac_samples_average_point( window, &point, 0 );
        gac_samples_average_screen_point( window, &screen_point, 0 );
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
            glm_vec3_copy( screen_point, filter->screen_point );
        }
        else if( filter->is_collecting )
        {
            // fixation stop
            gac_fixation_init( fixation, &filter->screen_point, &filter->point,
                    first_sample->timestamp, filter->duration,
                    first_sample->label );
            filter->is_collecting = false;
            if( action != NULL )
            {
                *action = GAC_FILTER_STEP_ACTION_CLEAR;
            }
            return true;
        }
        else if( action != NULL )
        {
            *action = GAC_FILTER_STEP_ACTION_SHRINK;
        }
    }

    return false;
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
    vec2 screen_point;
    vec3 point;
    vec3 origin;
    double factor;

    if( sample == NULL )
    {
        return 0;
    }

    if( !filter->is_enabled || samples->count == 0 )
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
        glm_vec2_lerp( last_sample->screen_point, sample->screen_point, factor,
                screen_point );
        new_sample = gac_sample_create( &screen_point, &origin, &point,
                last_sample->timestamp + ( i + 1 ) * filter->sample_period,
                sample->label );
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
        return NULL;
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
    vec2 screen_point;
    vec3 point;
    vec3 origin;

    gac_samples_average_screen_point( &filter->window, &screen_point, 0 );
    gac_samples_average_point( &filter->window, &point, 0 );
    gac_samples_average_origin( &filter->window, &origin, 0 );

    mid = filter->window.tail;
    while( mid != NULL && mid->next != NULL && count < filter->mid )
    {
        mid = mid->next;
        count++;
    }
    sample_mid = mid->data;

    return gac_sample_create( &screen_point, &origin, &point,
            sample_mid->timestamp, sample_mid->label );
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
bool gac_filter_saccade( gac_filter_saccade_t* filter, gac_sample_t* sample,
        gac_saccade_t* saccade )
{
    bool res;
    gac_filter_step_action_t action;

    gac_queue_push( &filter->window, sample );
    res = gac_filter_saccade_step( filter, sample, saccade, &action );

    switch( action )
    {
        case GAC_FILTER_STEP_ACTION_CLEAR:
            gac_queue_clear( &filter->window );
            break;
        case GAC_FILTER_STEP_ACTION_SHRINK:
            gac_queue_remove( &filter->window );
            break;
        default:
            break;
    }

    return res;
}

/******************************************************************************/
gac_filter_saccade_t* gac_filter_saccade_create( float velocity_threshold )
{
    gac_filter_saccade_t* filter = malloc( sizeof( gac_filter_saccade_t ) );
    if( !gac_filter_saccade_init( filter, velocity_threshold ) )
    {
        return NULL;
    }
    filter->is_heap = true;

    return filter;
}

/******************************************************************************/
void gac_filter_saccade_destroy( gac_filter_saccade_t* filter )
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
bool gac_filter_saccade_init( gac_filter_saccade_t* filter,
        float velocity_threshold )
{
    if( filter == NULL )
    {
        return false;
    }

    filter->is_heap = false;
    filter->is_collecting = false;
    filter->velocity_threshold = velocity_threshold;
    gac_queue_init( &filter->window, 0 );
    gac_queue_set_rm_handler( &filter->window, gac_sample_destroy );

    return true;
}

/******************************************************************************/
bool gac_filter_saccade_step( gac_filter_saccade_t* filter, gac_sample_t* sample,
        gac_saccade_t* saccade, gac_filter_step_action_t* action )
{
    gac_sample_t* s1;
    gac_sample_t* s2;
    vec3 v1;
    vec3 v2;
    double duration;
    float angle;
    float velocity;
    gac_queue_t* window;
    if( action != NULL )
    {
        *action = GAC_FILTER_STEP_ACTION_NONE;
    }

    if( filter == NULL || saccade == NULL || sample == NULL )
    {
        return false;
    }
    window = &filter->window;

    if( window->count < 2 )
    {
        return false;
    }

    s2 = window->tail->data;
    s1 = window->tail->next->data;
    duration = s2->timestamp - s1->timestamp;

    glm_vec3_sub( s1->point, s1->origin, v1 );
    glm_vec3_sub( s2->point, s2->origin, v2 );

    angle = glm_vec3_angle( v1, v2 ) * 180 / M_PI;
    velocity =  angle / ( duration / 1000 );

    if( velocity > filter->velocity_threshold )
    {
        if( !filter->is_collecting )
        {
            // saccade start
            filter->is_collecting = true;
        }
    }
    else if( filter->is_collecting )
    {
        // saccade stop
        s2 = s1;
        s1 = window->head->data;
        gac_saccade_init( saccade, &s1->screen_point, &s2->screen_point,
                &s1->point, &s2->point, s1->timestamp,
                s2->timestamp - s1->timestamp, s1->label );
        filter->is_collecting = false;
        if( action != NULL )
        {
            *action = GAC_FILTER_STEP_ACTION_CLEAR;
        }
        return true;
    }
    else if( action != NULL )
    {
        *action = GAC_FILTER_STEP_ACTION_SHRINK;
    }

    return false;
}

/******************************************************************************/
gac_fixation_t* gac_fixation_create( vec2* screen_point, vec3* point,
        double timestamp, double duration, const char* label )
{
    gac_fixation_t* fixation = malloc( sizeof( gac_fixation_t ) );
    if( !gac_fixation_init( fixation, screen_point, point, timestamp,
                duration, label ) )
    {
        return NULL;
    }
    fixation->is_heap = true;

    return fixation;
}

/******************************************************************************/
void gac_fixation_destroy( gac_fixation_t* fixation )
{
    if( fixation == NULL )
    {
        return;
    }

    if( fixation->label != NULL )
    {
        free( fixation->label );
    }

    if( fixation->is_heap )
    {
        free( fixation );
    }
}

/******************************************************************************/
bool gac_fixation_init( gac_fixation_t* fixation, vec2* screen_point,
        vec3* point, double timestamp, double duration, const char* label )
{
    if( fixation == NULL )
    {
        return false;
    }

    fixation->is_heap = false;
    fixation->duration = duration;
    glm_vec2_copy( *screen_point, fixation->screen_point );
    glm_vec3_copy( *point, fixation->point );
    fixation->timestamp = timestamp;
    fixation->label = NULL;
    if( label != NULL )
    {
        fixation->label = strdup( label );
    }

    return true;
}

/******************************************************************************/
float gac_fixation_normalised_dispersion_threshold( float angle )
{
    return sqrt( 2 * ( 1 - cos( angle * M_PI / 180 ) ) );
}

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
    if( queue == NULL )
    {
        return false;
    }

    queue->rm = rm;

    return true;
}

/******************************************************************************/
gac_saccade_t* gac_saccade_create( vec2* screen_point_start,
        vec2* screen_point_dest, vec3* point_start, vec3* point_dest,
        double timestamp, double duration, const char* label )
{
    gac_saccade_t* saccade = malloc( sizeof( gac_fixation_t ) );
    if( !gac_saccade_init( saccade, screen_point_start, screen_point_dest,
                point_start, point_dest, timestamp, duration, label ) )
    {
        return NULL;
    }
    saccade->is_heap = true;

    return saccade;
}

/******************************************************************************/
void gac_saccade_destroy( gac_saccade_t* saccade )
{
    if( saccade == NULL )
    {
        return;
    }

    if( saccade->label != NULL )
    {
        free( saccade->label );
    }

    if( saccade->is_heap )
    {
        free( saccade );
    }
}

/******************************************************************************/
bool gac_saccade_init( gac_saccade_t* saccade, vec2* screen_point_start,
        vec2* screen_point_dest, vec3* point_start, vec3* point_dest,
        double timestamp, double duration, const char* label )
{
    if( saccade == NULL )
    {
        return false;
    }

    saccade->is_heap = false;
    saccade->duration = duration;
    glm_vec2_copy( *screen_point_start, saccade->screen_point_start );
    glm_vec2_copy( *screen_point_dest, saccade->screen_point_dest );
    glm_vec3_copy( *point_start, saccade->point_start );
    glm_vec3_copy( *point_dest, saccade->point_dest );
    saccade->timestamp = timestamp;
    saccade->label = NULL;
    if( label != NULL )
    {
        saccade->label = strdup( label );
    }

    return true;
}

/******************************************************************************/
gac_sample_t* gac_sample_create( vec2* screen_point, vec3* origin, vec3* point,
        double timestamp, const char* label )
{
    gac_sample_t* sample = malloc( sizeof( gac_sample_t ) );
    if( !gac_sample_init( sample, screen_point, origin, point, timestamp, label ) )
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

    if( sample->label != NULL )
    {
        free( sample->label );
    }

    if( sample->is_heap )
    {
        free( sample );
    }
}

/******************************************************************************/
bool gac_sample_init( gac_sample_t* sample, vec2* screen_point, vec3* origin,
        vec3* point, double timestamp, const char* label )
{
    if( sample == NULL )
    {
        return false;
    }

    sample->is_heap = false;
    sample->label = NULL;
    if( label != NULL )
    {
        sample->label = strdup( label );
    }
    glm_vec3_copy( *screen_point, sample->screen_point );
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
bool gac_sample_window_fixation_filter( gac_t* h, gac_fixation_t* fixation )
{
    gac_sample_t* sample;
    gac_filter_step_action_t action;
    bool res;

    if( fixation == NULL || h == NULL || h->samples.count == 0 )
    {
        return false;
    }

    sample = h->samples.tail->data;

    gac_queue_push( &h->fixation.window, sample );
    res = gac_filter_fixation_step( &h->fixation, sample, fixation, &action );

    switch( action )
    {
        case GAC_FILTER_STEP_ACTION_CLEAR:
            gac_queue_clear( &h->fixation.window );
            break;
        case GAC_FILTER_STEP_ACTION_SHRINK:
            gac_queue_remove( &h->fixation.window );
            break;
        default:
            break;
    }

    return res;
}

/******************************************************************************/
bool gac_sample_window_saccade_filter( gac_t* h, gac_saccade_t* saccade )
{
    gac_filter_step_action_t action;
    gac_sample_t* sample;
    bool res;

    if( h == NULL || saccade == NULL || h->samples.count == 0 )
    {
        return false;
    }

    sample = h->samples.tail->data;

    gac_queue_push( &h->saccade.window, sample );
    res = gac_filter_saccade_step( &h->saccade, sample, saccade, &action );

    switch( action )
    {
        case GAC_FILTER_STEP_ACTION_CLEAR:
            gac_queue_clear( &h->saccade.window );
            break;
        case GAC_FILTER_STEP_ACTION_SHRINK:
            gac_queue_remove( &h->saccade.window );
            break;
        default:
            break;
    }

    return res;
}

/******************************************************************************/
int gac_sample_window_update( gac_t* h, float ox, float oy, float oz,
        float px, float py, float pz, double timestamp, const char* label )
{
    vec2 screen_point;
    vec3 point;
    vec3 origin;

    origin[0] = ox;
    origin[1] = oy;
    origin[2] = oz;
    point[0] = px;
    point[1] = py;
    point[2] = pz;

    if( h->screen != NULL )
    {
        if( h->is_normalized )
        {
            gac_screen_point_normalized( h->screen, &point, &screen_point );
        }
        else
        {
            gac_screen_point( h->screen, &point, &screen_point );
        }
    }
    else
    {
        glm_vec2_zero( screen_point );
    }

    return gac_sample_window_update_vec( h, &screen_point, &origin, &point,
            timestamp, label );
}

/******************************************************************************/
int gac_sample_window_update_vec( gac_t* h, vec2* screen_point, vec3* origin,
        vec3* point, double timestamp, const char* label )
{
    gac_sample_t* sample;

    sample = gac_sample_create( screen_point, origin, point, timestamp, label );

    sample = gac_filter_noise( &h->noise, sample );
    return gac_filter_gap( &h->gap, &h->samples, sample );
}

/******************************************************************************/
int gac_sample_window_update_screen( gac_t* h, float ox, float oy, float oz,
        float px, float py, float pz, float sx, float sy, double timestamp,
        const char* label )
{
    vec2 screen_point;
    vec3 point;
    vec3 origin;

    screen_point[0] = sx;
    screen_point[1] = sy;
    origin[0] = ox;
    origin[1] = oy;
    origin[2] = oz;
    point[0] = px;
    point[1] = py;
    point[2] = pz;

    return gac_sample_window_update_vec( h, &screen_point, &origin, &point,
            timestamp, label );
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
            goto success;
        }
    }

    if( count > 0 && i < count )
    {
        return false;
    }

success:
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
            goto success;
        }
    }

    if( count > 0 && i < count )
    {
        return false;
    }

success:
    glm_vec3_divs( *avg, samples->count, *avg );
    return true;
}

/******************************************************************************/
bool gac_samples_average_screen_point( gac_queue_t* samples, vec2* avg,
        uint32_t count )
{
    uint32_t i = 0;
    gac_sample_t* sample;
    gac_queue_item_t* item = samples->tail;

    if( avg == NULL || samples->count == 0 )
    {
        return false;
    }

    glm_vec2_zero( *avg );

    while( item != NULL )
    {
        sample = item->data;
        glm_vec2_add( *avg, sample->screen_point, *avg );
        item = item->next;

        i++;
        if( i == count )
        {
            goto success;
        }
    }

    if( count > 0 && i < count )
    {
        return false;
    }

success:
    glm_vec2_divs( *avg, samples->count, *avg );
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
    glm_vec3_zero( min );
    glm_vec3_zero( max );

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
            goto success;
        }
    }

    if( count > 0 && i < count )
    {
        printf("count(%d) > 0, i(%d) < count(%d), item: %p:\n", count, i, count, item );
        return false;
    }

success:
    *dispersion = max[0] - min[0] + max[1] - min[1] + max[2] - min[2];
    return true;
}

/******************************************************************************/
gac_screen_t* gac_screen_create( vec3* top_left, vec3* top_right,
        vec3* bottom_left, vec3* bottom_right )
{
    gac_screen_t* screen = malloc( sizeof( gac_screen_t ) );

    if( screen == NULL )
    {
        return NULL;
    }

    if( !gac_screen_init( screen, top_left, top_right, bottom_left,
                bottom_right ) )
    {
        gac_screen_destroy( screen );
        return NULL;
    }

    return screen;
}

/******************************************************************************/
void gac_screen_destroy( gac_screen_t* screen )
{
    if( screen == NULL || screen->is_heap)
    {
        return;
    }

    free( screen );
}

/******************************************************************************/
bool gac_screen_init( gac_screen_t* screen, vec3* top_left, vec3* top_right,
        vec3* bottom_left, vec3* bottom_right )
{
    mat4 d = {
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f}
    };
    mat4 s = {
        {0.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 0.0f},
        {1.0f, 1.0f, 1.0f, 1.0f}
    };
    mat4 s_inv;
    vec3 e1, e2, norm;
    vec3 u, v, w;

    if( screen == NULL || top_left == NULL || top_right == NULL
            || bottom_left == NULL || bottom_right == NULL )
    {
        return false;
    }

    glm_vec3_copy( *top_left, screen->top_left );
    glm_vec3_copy( *top_right, screen->top_right );
    glm_vec3_copy( *bottom_left, screen->bottom_left );
    glm_vec3_copy( *bottom_right, screen->bottom_right );

    glm_vec3_sub( *top_right, *top_left, e1 );
    glm_vec3_sub( *bottom_left, *top_left, e2 );
    glm_vec3_cross( e1, e2, screen->norm );

    screen->width = glm_vec3_norm( e1 );
    screen->height = glm_vec3_norm( e2 );

    glm_vec3_normalize( e1 );
    glm_vec3_normalize( e2 );
    glm_vec3_normalize_to( screen->norm, norm );
    glm_vec3_add( *top_left, e1, u );
    glm_vec3_add( *top_left, e2, v );
    glm_vec3_add( *top_left, norm, w );

    s[0][0] = ( *top_left )[0];
    s[1][0] = ( *top_left )[1];
    s[2][0] = ( *top_left )[2];
    s[0][1] = u[0];
    s[1][1] = u[1];
    s[2][1] = u[2];
    s[0][2] = v[0];
    s[1][2] = v[1];
    s[2][2] = v[2];
    s[0][3] = w[0];
    s[1][3] = w[1];
    s[2][3] = w[2];

    glm_mat4_inv( s, s_inv );
    glm_mat4_mul( d, s_inv, screen->m );
    glm_mat4_transpose( screen->m );
    gac_screen_point( screen, top_left, &screen->origin );

    return true;
}

/******************************************************************************/
bool gac_screen_intersection( gac_screen_t* screen, vec3* origin, vec3* dir,
        vec3* intersection )
{
    float d;
    float n;
    vec3 dir_scale, dir_neg, dir_init;

    if( screen == NULL || origin == NULL || dir == NULL
            || intersection == NULL )
    {
        return false;
    }

    glm_vec3_scale( *dir, -1, dir_neg );
    glm_vec3_sub( *origin, screen->top_left, dir_init );

    d = glm_vec3_dot( screen->norm, dir_init );
    n = glm_vec3_dot( screen->norm, dir_neg );

    if( n == 0 )
    {
        return false;
    }

    glm_vec3_scale( *dir, d / n, dir_scale );
    glm_vec3_add( *origin, dir_scale, *intersection );

    return true;
}

/******************************************************************************/
bool gac_screen_point( gac_screen_t* screen, vec3* point3d, vec2* point2d )
{
    vec3 p;

    if( screen == NULL || point3d == NULL || point2d == NULL )
    {
        return false;
    }

    glm_mat4_mulv3( screen->m, *point3d, 1, p );
    ( *point2d )[0] = p[0];
    ( *point2d )[1] = p[1];

    return true;
}

/******************************************************************************/
bool gac_screen_point_normalized( gac_screen_t* screen, vec3* point3d,
        vec2* point2d )
{
    vec2 p_offset, p;

    if( !gac_screen_point( screen, point3d, &p_offset ) )
    {
        return false;
    }

    glm_vec2_sub( p_offset, screen->origin, p );
    ( *point2d )[0] = p[0] / screen->width;
    ( *point2d )[1] = p[1] / screen->height;

    return true;
}
