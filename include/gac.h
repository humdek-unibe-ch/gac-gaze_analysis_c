
#ifndef GAC_H
#define GAC_H

#include <stdint.h>
#include <cglm/cglm.h>
#include <sys/time.h>

typedef struct gac_s gac_t;
typedef struct gac_filter_fixation_s gac_filter_fixation_t;
typedef struct gac_filter_gap_s gac_filter_gap_t;
typedef struct gac_filter_noise_s gac_filter_noise_t;
typedef struct gac_filter_saccade_s gac_filter_saccade_t;
typedef struct gac_filter_parameter_s gac_filter_parameter_t;
typedef struct gac_fixation_s gac_fixation_t;
typedef struct gac_saccade_s gac_saccade_t;
typedef struct gac_sample_s gac_sample_t;
typedef struct gac_queue_s gac_queue_t;
typedef struct gac_queue_item_s gac_queue_item_t;

typedef enum gac_filter_noise_type_e gac_filter_noise_type_t;
typedef enum gac_fixation_step_action_e gac_fixation_step_action_t;

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

enum gac_fixation_step_action_e
{
    GAC_FIXATION_STEP_ACTION_SHRINK,
    GAC_FIXATION_STEP_ACTION_CLEAR,
    GAC_FIXATION_STEP_ACTION_NONE
};

/**
 * The gaze data sample.
 */
struct gac_sample_s
{
    bool is_heap;
    /** The gaze point. */
    vec3 point;
    /** The gaze origin. */
    vec3 origin;
    /** The sample timestamp. */
    double timestamp;
};

/**
 * A fixation sample.
 */
struct gac_fixation_s
{
    bool is_heap;
    /** The fixation gaze point */
    vec3 point;
    /** The fixation duration in milliseconds */
    double duration;
    /** The timestamp of the fixation start */
    double timestamp;
};

/**
 * A saccade sample.
 */
struct gac_saccade_s
{
    bool is_heap;
    /** The start point of the saccade */
    vec3 point_start;
    /** The end point of the saccade */
    vec3 point_dest;
    /** The sacacde duration */
    double duration;
    /** The timestamp of the first saccade point */
    double timestamp;
};

/**
 * A generic queue structure.
 */
struct gac_queue_s
{
    bool is_heap;
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
 * The fixation filter structure holding filter parameters.
 */
struct gac_filter_fixation_s
{
    bool is_heap;
    /** The pre-computed dispersion threshold at unit distance */
    double normalized_dispersion_threshold;
    /** The duration threashold */
    double duration_threshold;
    /** A flag indicating whether a fixation is ongoing */
    bool is_collecting;
    /** A pointer to the sample queue */
    gac_queue_t window;
    /** The fixation duration */
    double duration;
    /** The fixation point */
    vec3 point;
};

/**
 * The saccade filter structure holding filter parameters.
 */
struct gac_filter_saccade_s
{
    bool is_heap;
    /** The velocity threshold */
    float velocity_threshold;
    /** A flag indicating whether a saccade is ongoing */
    bool is_collecting;
    /** A pointer to the sample queue */
    gac_queue_t window;
    /** The saccade duration */
    double duration;
};

/**
 * The noise filter parameters.
 */
struct gac_filter_noise_s
{
    bool is_heap;
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
    bool is_heap;
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
    bool is_heap;
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
 * The gaze analysis handler structure.
 */
struct gac_s
{
    bool is_heap;
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
 *
 */
bool gac_filter_fixation_step( gac_filter_fixation_t* filter,
        gac_sample_t* sample, gac_fixation_t* fixation,
        gac_fixation_step_action_t* action );

/**
 * Allocate a new fixation filter structure on the heap. This structure must be
 * freed.
 *
 * @param dispersion_threshold
 *  The dispersion thresholad in degrees.
 * @apram duratio_threshold
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
 * @apram duratio_threshold
 *  The duration threshold in milliseconds.
 * @return
 *  True on success, false on failure.
 */
bool gac_filter_fixation_init( gac_filter_fixation_t* filter,
        float dispersion_threshold, double duration_threshold );

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
 *  window: window_length = mid_idx * 2 + 1.
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
 *  window: window_length = mid_idx * 2 + 1.
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

// FIXATION ////////////////////////////////////////////////////////////////////

/**
 * Allocate a new fixation structure on the heap. This structure must be
 * freed.
 *
 * @param point
 *  The fixation point.
 * @param timestamp
 *  The timestamp of the fixation start.
 * @param duration
 *  The duration of the fixation.
 * @return
 *  The allocated fixation structure or NULL on failure.
 */
gac_fixation_t* gac_fixation_create( vec3* point, double timestamp,
        double duration );

/**
 * The fixation detection algorithm I-DT. This acts on the sample window managed
 * by the functions gac_sample_add() and gac_sample_cleanup().
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
 * Initialise a fixation structure.
 *
 * @param fixation
 *  The fixation structure to initialise.
 * @param point
 *  The fixation point.
 * @param timestamp
 *  The timestamp of the fixation start.
 * @param duration
 *  The duration of the fixation.
 * @return
 *  True on success, false on failure.
 */
bool gac_fixation_init( gac_fixation_t* fixation, vec3* point,
        double timestamp, double duration );

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

// QUEUE ///////////////////////////////////////////////////////////////////////

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

// SACCADE /////////////////////////////////////////////////////////////////////

/**
 * Allocate a new saccade structure on the heap. This needs to be freed.
 *
 * @param point_start
 *  The first data point in a saccade.
 * @param point_dest
 *  The last data point in a saccade.
 * @param timestamp
 *  The timestamp of the beggining of the saccade.
 * @param duration
 *  The duration of the saccade.
 * @return
 *  The allocated saccade structure on success or NULL on failure.
 */
gac_saccade_t* gac_saccade_create( vec3* point_start, vec3* point_dest,
        double timestamp, double duration );

/**
 * The saccade detection algorithm I-VT. This acts on the sample window managed
 * by the functions gac_sample_add() and gac_sample_cleanup().
 *
 * @param h
 *  A pointer to the gaze analysis handler.
 * @param saccade
 *  A location where a detected saccade is stored. This is only valid if the
 *  function returns true.
 * @return
 *  True if a saccade was detected, false otherwise.
 */
bool gac_saccade_filter( gac_t* h, gac_saccade_t* saccade );

/**
 * Allocate a new saccade filter structure on the heap. This needs to be freed.
 *
 * @param velocity_thresold
 *  The velocity threshold in degrees per second.
 * @return
 *  A pointer to the allocated filter structure or NUll on failure.
 */
gac_filter_saccade_t* gac_saccade_filter_create( float velocity_threshold );

/**
 * Destroy the saccade filter structure.
 *
 * @param filter
 *  A pointer to the structure to destroy.
 */
void gac_saccade_filter_destroy( gac_filter_saccade_t* filter );

/**
 * Initialise a saccade filter structure.
 *
 * @param filter
 *  A pointer to the filter structure to initialise.
 * @param velocity_thresold
 *  The velocity threshold in degrees per second.
 * @return
 *  True on success, false on failure.
 */
bool gac_saccade_filter_init( gac_filter_saccade_t* filter,
        float velocity_threshold );

/**
 * Initialise a saccade structure.
 *
 * @param saccade
 *  A pointer to the saccade structure to initialise.
 * @param point_start
 *  The first data point in a saccade.
 * @param point_dest
 *  The last data point in a saccade.
 * @param timestamp
 *  The timestamp of the beggining of the saccade.
 * @param duration
 *  The duration of the saccade.
 * @return
 *  True on success, false on failure.
 */
bool gac_saccade_init( gac_saccade_t* saccade, vec3* point_start,
        vec3* point_dest, double timestamp, double duration );

// SAMPLE //////////////////////////////////////////////////////////////////////

/**
 * Allocate a new sample structure on the heap. This needs to be freed.
 *
 * @param origin
 *  The gaze origin vector.
 * @param point
 *  The gaze point vector.
 * @param timestamp
 *  The timestamp of the sample.
 * @return
 *  The allocated sample structure or NULL on failure.
 */
gac_sample_t* gac_sample_create( vec3* origin, vec3* point, double timestamp );

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
 * @param origin
 *  The gaze origin vector.
 * @param point
 *  The gaze point vector.
 * @param timestamp
 *  The timestamp of the sample.
 * @return
 *  True on success, false on failure.
 */
bool gac_sample_init( gac_sample_t* sample, vec3* origin, vec3* point,
        double timestamp );

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
 * @return
 *  True on success, false on failure.
 */
uint32_t gac_sample_window_update( gac_t* h, float ox, float oy, float oz,
        float px, float py, float pz, double timestamp );

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

#endif
