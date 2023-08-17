#include "minunit.h"
#include "gac.h"

#undef MINUNIT_EPSILON
#define MINUNIT_EPSILON 1E-7

static gac_filter_gap_t gap_stack;
static gac_filter_gap_t* gap_heap;
static gac_filter_gap_t* gap;
static double max_gap_length = 50;
static double sample_period = 1000.0/60.0;
static gac_queue_t samples_stack;
static gac_queue_t* samples_heap;
static gac_queue_t* samples;

static float s[2] = { 0, 0 };

float lerp( float a, float b, float t )
{
    return a + ( b - a ) * t;
}

void gap_setup()
{
    gac_filter_gap_init( &gap_stack, max_gap_length, sample_period );
    gap = &gap_stack;
    gac_queue_init( &samples_stack, 0 );
    samples = &samples_stack;
}

void gap_teardown()
{
    gac_filter_gap_destroy( gap );
    gac_queue_destroy( samples );
}

MU_TEST( gap_init_stack )
{
    gac_filter_gap_init( &gap_stack, max_gap_length, sample_period );
    gap = &gap_stack;
    gac_queue_init( &samples_stack, 0 );
    samples = &samples_stack;
}

MU_TEST( gap_init_heap )
{
    gap_heap = gac_filter_gap_create( max_gap_length, sample_period );
    gap = gap_heap;
    samples_heap = gac_queue_create( 0 );
    samples = samples_heap;
}

MU_TEST_SUITE( gap_init_suite )
{
    MU_SUITE_CONFIGURE( NULL, &gap_teardown );
    MU_RUN_TEST( gap_init_stack );
    MU_RUN_TEST( gap_init_heap );
}

MU_TEST( gap_1 )
{
    unsigned int count;

    double timestamp = 1000;
    float o[3] = { 0.1, 0.2, 0.3 };
    float p[3] = { 0.4, 0.5, 0.6 };

    gac_sample_t* sample = gac_sample_create( &s, &o, &p, timestamp, 0, NULL );
    count = gac_filter_gap( gap, samples, sample );
    mu_assert_int_eq( 1, count );
    mu_assert_int_eq( 1, samples->count );

    gac_queue_pop( samples, ( void** )&sample );
    mu_assert_double_eq( o[0], sample->origin[0] );
    mu_assert_double_eq( o[1], sample->origin[1] );
    mu_assert_double_eq( o[2], sample->origin[2] );
    mu_assert_double_eq( p[0], sample->point[0] );
    mu_assert_double_eq( p[1], sample->point[1] );
    mu_assert_double_eq( p[2], sample->point[2] );
    mu_assert_double_eq( timestamp, sample->timestamp );
    free( sample );
}

MU_TEST( gap_2 )
{
    unsigned int count;

    double timestamp = 1000;
    float o[3] = { 0.1, 0.2, 0.3 };
    float p[3] = { 0.4, 0.5, 0.6 };

    double timestamp2= 1016.6666667;
    float o2[3] = { 0.2, 0.3, 0.4 };
    float p2[3] = { 0.5, 0.6, 0.7 };

    gac_sample_t* sample = gac_sample_create( &s, &o, &p, timestamp, 0, NULL );
    count = gac_filter_gap( gap, samples, sample );
    mu_assert_int_eq( count, 1 );
    mu_assert_int_eq( samples->count, 1 );

    sample = gac_sample_create( &s, &o2, &p2, timestamp2, 0, NULL );
    count = gac_filter_gap( gap, samples, sample );
    mu_assert_int_eq( count, 1 );
    mu_assert_int_eq( samples->count, 2 );

    gac_queue_pop( samples, ( void** )&sample );
    mu_assert_double_eq( o[0], sample->origin[0] );
    mu_assert_double_eq( o[1], sample->origin[1] );
    mu_assert_double_eq( o[2], sample->origin[2] );
    mu_assert_double_eq( p[0], sample->point[0] );
    mu_assert_double_eq( p[1], sample->point[1] );
    mu_assert_double_eq( p[2], sample->point[2] );
    mu_assert_double_eq( timestamp, sample->timestamp );
    free( sample );

    gac_queue_pop( samples, ( void** )&sample );
    mu_assert_double_eq( o2[0], sample->origin[0] );
    mu_assert_double_eq( o2[1], sample->origin[1] );
    mu_assert_double_eq( o2[2], sample->origin[2] );
    mu_assert_double_eq( p2[0], sample->point[0] );
    mu_assert_double_eq( p2[1], sample->point[1] );
    mu_assert_double_eq( p2[2], sample->point[2] );
    mu_assert_double_eq( timestamp2, sample->timestamp );
    free( sample );
}

MU_TEST( gap_2_plus )
{
    unsigned int count;

    double timestamp = 1000;
    float o[3] = { 0.1, 0.2, 0.3 };
    float p[3] = { 0.4, 0.5, 0.6 };

    double timestamp2= 1018;
    float o2[3] = { 0.2, 0.3, 0.4 };
    float p2[3] = { 0.5, 0.6, 0.7 };

    gac_sample_t* sample = gac_sample_create( &s, &o, &p, timestamp, 0, NULL );
    count = gac_filter_gap( gap, samples, sample );
    mu_assert_int_eq( count, 1 );
    mu_assert_int_eq( samples->count, 1 );

    sample = gac_sample_create( &s, &o2, &p2, timestamp2, 0, NULL );
    count = gac_filter_gap( gap, samples, sample );
    mu_assert_int_eq( count, 1 );
    mu_assert_int_eq( samples->count, 2 );

    gac_queue_pop( samples, ( void** )&sample );
    mu_assert_double_eq( o[0], sample->origin[0] );
    mu_assert_double_eq( o[1], sample->origin[1] );
    mu_assert_double_eq( o[2], sample->origin[2] );
    mu_assert_double_eq( p[0], sample->point[0] );
    mu_assert_double_eq( p[1], sample->point[1] );
    mu_assert_double_eq( p[2], sample->point[2] );
    mu_assert_double_eq( timestamp, sample->timestamp );
    free( sample );

    gac_queue_pop( samples, ( void** )&sample );
    mu_assert_double_eq( o2[0], sample->origin[0] );
    mu_assert_double_eq( o2[1], sample->origin[1] );
    mu_assert_double_eq( o2[2], sample->origin[2] );
    mu_assert_double_eq( p2[0], sample->point[0] );
    mu_assert_double_eq( p2[1], sample->point[1] );
    mu_assert_double_eq( p2[2], sample->point[2] );
    mu_assert_double_eq( timestamp2, sample->timestamp );
    free( sample );
}

MU_TEST( gap_2_minus )
{
    unsigned int count;

    double timestamp = 1000;
    float o[3] = { 0.1, 0.2, 0.3 };
    float p[3] = { 0.4, 0.5, 0.6 };

    double timestamp2= 1015;
    float o2[3] = { 0.2, 0.3, 0.4 };
    float p2[3] = { 0.5, 0.6, 0.7 };

    gac_sample_t* sample = gac_sample_create( &s, &o, &p, timestamp, 0, NULL );
    count = gac_filter_gap( gap, samples, sample );
    mu_assert_int_eq( count, 1 );
    mu_assert_int_eq( samples->count, 1 );

    sample = gac_sample_create( &s, &o2, &p2, timestamp2, 0, NULL );
    count = gac_filter_gap( gap, samples, sample );
    mu_assert_int_eq( count, 1 );
    mu_assert_int_eq( samples->count, 2 );

    gac_queue_pop( samples, ( void** )&sample );
    mu_assert_double_eq( o[0], sample->origin[0] );
    mu_assert_double_eq( o[1], sample->origin[1] );
    mu_assert_double_eq( o[2], sample->origin[2] );
    mu_assert_double_eq( p[0], sample->point[0] );
    mu_assert_double_eq( p[1], sample->point[1] );
    mu_assert_double_eq( p[2], sample->point[2] );
    mu_assert_double_eq( timestamp, sample->timestamp );
    free( sample );

    gac_queue_pop( samples, ( void** )&sample );
    mu_assert_double_eq( o2[0], sample->origin[0] );
    mu_assert_double_eq( o2[1], sample->origin[1] );
    mu_assert_double_eq( o2[2], sample->origin[2] );
    mu_assert_double_eq( p2[0], sample->point[0] );
    mu_assert_double_eq( p2[1], sample->point[1] );
    mu_assert_double_eq( p2[2], sample->point[2] );
    mu_assert_double_eq( timestamp2, sample->timestamp );
    free( sample );
}

MU_TEST( gap_2_max )
{
    unsigned int count;

    double timestamp = 1000;
    float o[3] = { 0.1, 0.2, 0.3 };
    float p[3] = { 0.4, 0.5, 0.6 };

    double timestamp2= 1051;
    float o2[3] = { 0.2, 0.3, 0.4 };
    float p2[3] = { 0.5, 0.6, 0.7 };

    gac_sample_t* sample = gac_sample_create( &s, &o, &p, timestamp, 0, NULL );
    count = gac_filter_gap( gap, samples, sample );
    mu_assert_int_eq( count, 1 );
    mu_assert_int_eq( samples->count, 1 );

    sample = gac_sample_create( &s, &o2, &p2, timestamp2, 0, NULL );
    count = gac_filter_gap( gap, samples, sample );
    mu_assert_int_eq( count, 1 );
    mu_assert_int_eq( samples->count, 2 );

    gac_queue_pop( samples, ( void** )&sample );
    mu_assert_double_eq( o[0], sample->origin[0] );
    mu_assert_double_eq( o[1], sample->origin[1] );
    mu_assert_double_eq( o[2], sample->origin[2] );
    mu_assert_double_eq( p[0], sample->point[0] );
    mu_assert_double_eq( p[1], sample->point[1] );
    mu_assert_double_eq( p[2], sample->point[2] );
    mu_assert_double_eq( timestamp, sample->timestamp );
    free( sample );

    gac_queue_pop( samples, ( void** )&sample );
    mu_assert_double_eq( o2[0], sample->origin[0] );
    mu_assert_double_eq( o2[1], sample->origin[1] );
    mu_assert_double_eq( o2[2], sample->origin[2] );
    mu_assert_double_eq( p2[0], sample->point[0] );
    mu_assert_double_eq( p2[1], sample->point[1] );
    mu_assert_double_eq( p2[2], sample->point[2] );
    mu_assert_double_eq( timestamp2, sample->timestamp );
    free( sample );
}

MU_TEST( gap_2_fill_1 )
{
    unsigned int count;
    gac_sample_t* sample;
    double timestamp = 1000;
    float o[3] = { 0.1, 0.2, 0.3 };
    float p[3] = { 0.4, 0.5, 0.6 };
    gac_sample_t* sample1 = gac_sample_create( &s, &o, &p, timestamp, 0, NULL );

    double timestamp2= 1033.333333;
    float o2[3] = { 0.2, 0.3, 0.4 };
    float p2[3] = { 0.5, 0.6, 0.7 };
    gac_sample_t* sample2 = gac_sample_create( &s, &o2, &p2, timestamp2, 0, NULL );

    count = gac_filter_gap( gap, samples, sample1 );
    mu_assert_int_eq( count, 1 );
    mu_assert_int_eq( samples->count, 1 );

    count = gac_filter_gap( gap, samples, sample2 );
    mu_assert_int_eq( count, 2 );
    mu_assert_int_eq( samples->count, 3 );

    gac_queue_pop( samples, ( void** )&sample );
    mu_assert_double_eq( o[0], sample->origin[0] );
    mu_assert_double_eq( o[1], sample->origin[1] );
    mu_assert_double_eq( o[2], sample->origin[2] );
    mu_assert_double_eq( p[0], sample->point[0] );
    mu_assert_double_eq( p[1], sample->point[1] );
    mu_assert_double_eq( p[2], sample->point[2] );
    mu_assert_double_eq( timestamp, sample->timestamp );
    free( sample );

    gac_queue_pop( samples, ( void** )&sample );
    mu_assert_double_eq( lerp( o[0], o2[0], 1.0 / 2 ), sample->origin[0] );
    mu_assert_double_eq( lerp( o[1], o2[1], 1.0 / 2 ), sample->origin[1] );
    mu_assert_double_eq( lerp( o[2], o2[2], 1.0 / 2 ), sample->origin[2] );
    mu_assert_double_eq( lerp( p[0], p2[0], 1.0 / 2 ), sample->point[0] );
    mu_assert_double_eq( lerp( p[1], p2[1], 1.0 / 2 ), sample->point[1] );
    mu_assert_double_eq( lerp( p[2], p2[2], 1.0 / 2 ), sample->point[2] );
    mu_assert_double_eq( timestamp + sample_period, sample->timestamp );
    free( sample );

    gac_queue_pop( samples, ( void** )&sample );
    mu_assert_double_eq( o2[0], sample->origin[0] );
    mu_assert_double_eq( o2[1], sample->origin[1] );
    mu_assert_double_eq( o2[2], sample->origin[2] );
    mu_assert_double_eq( p2[0], sample->point[0] );
    mu_assert_double_eq( p2[1], sample->point[1] );
    mu_assert_double_eq( p2[2], sample->point[2] );
    mu_assert_double_eq( timestamp2, sample->timestamp );
    free( sample );
}

MU_TEST( gap_2_fill_2 )
{
    unsigned int count;
    gac_sample_t* sample;
    double timestamp = 1000;
    float o[3] = { 0.1, 0.2, 0.3 };
    float p[3] = { 0.4, 0.5, 0.6 };
    gac_sample_t* sample1 = gac_sample_create( &s, &o, &p, timestamp, 0, NULL );

    double timestamp2= 1050;
    float o2[3] = { 0.2, 0.3, 0.4 };
    float p2[3] = { 0.5, 0.6, 0.7 };
    gac_sample_t* sample2 = gac_sample_create( &s, &o2, &p2, timestamp2, 0, NULL );

    count = gac_filter_gap( gap, samples, sample1 );
    mu_assert_int_eq( count, 1 );
    mu_assert_int_eq( samples->count, 1 );

    count = gac_filter_gap( gap, samples, sample2 );
    mu_assert_int_eq( count, 3 );
    mu_assert_int_eq( samples->count, 4 );

    gac_queue_pop( samples, ( void** )&sample );
    mu_assert_double_eq( o[0], sample->origin[0] );
    mu_assert_double_eq( o[1], sample->origin[1] );
    mu_assert_double_eq( o[2], sample->origin[2] );
    mu_assert_double_eq( p[0], sample->point[0] );
    mu_assert_double_eq( p[1], sample->point[1] );
    mu_assert_double_eq( p[2], sample->point[2] );
    mu_assert_double_eq( timestamp, sample->timestamp );
    free( sample );

    gac_queue_pop( samples, ( void** )&sample );
    mu_assert_double_eq( lerp( o[0], o2[0], 1.0 / 3 ), sample->origin[0] );
    mu_assert_double_eq( lerp( o[1], o2[1], 1.0 / 3 ), sample->origin[1] );
    mu_assert_double_eq( lerp( o[2], o2[2], 1.0 / 3 ), sample->origin[2] );
    mu_assert_double_eq( lerp( p[0], p2[0], 1.0 / 3 ), sample->point[0] );
    mu_assert_double_eq( lerp( p[1], p2[1], 1.0 / 3 ), sample->point[1] );
    mu_assert_double_eq( lerp( p[2], p2[2], 1.0 / 3 ), sample->point[2] );
    mu_assert_double_eq( timestamp + sample_period, sample->timestamp );
    free( sample );

    gac_queue_pop( samples, ( void** )&sample );
    mu_assert_double_eq( lerp( o[0], o2[0], 2.0 / 3 ), sample->origin[0] );
    mu_assert_double_eq( lerp( o[1], o2[1], 2.0 / 3 ), sample->origin[1] );
    mu_assert_double_eq( lerp( o[2], o2[2], 2.0 / 3 ), sample->origin[2] );
    mu_assert_double_eq( lerp( p[0], p2[0], 2.0 / 3 ), sample->point[0] );
    mu_assert_double_eq( lerp( p[1], p2[1], 2.0 / 3 ), sample->point[1] );
    mu_assert_double_eq( lerp( p[2], p2[2], 2.0 / 3 ), sample->point[2] );
    mu_assert_double_eq( timestamp + 2 * sample_period, sample->timestamp );
    free( sample );

    gac_queue_pop( samples, ( void** )&sample );
    mu_assert_double_eq( o2[0], sample->origin[0] );
    mu_assert_double_eq( o2[1], sample->origin[1] );
    mu_assert_double_eq( o2[2], sample->origin[2] );
    mu_assert_double_eq( p2[0], sample->point[0] );
    mu_assert_double_eq( p2[1], sample->point[1] );
    mu_assert_double_eq( p2[2], sample->point[2] );
    mu_assert_double_eq( timestamp2, sample->timestamp );
    free( sample );
}

void gap_run()
{
    MU_RUN_TEST( gap_1 );
    MU_RUN_TEST( gap_2 );
    MU_RUN_TEST( gap_2_max );
    MU_RUN_TEST( gap_2_plus );
    MU_RUN_TEST( gap_2_minus );
    MU_RUN_TEST( gap_2_fill_1 );
    MU_RUN_TEST( gap_2_fill_2 );
}

MU_TEST_SUITE( gap_suite )
{
    MU_SUITE_CONFIGURE( &gap_setup, &gap_teardown );
    gap_run();
}

int main()
{
    MU_RUN_SUITE( gap_init_suite );
    MU_RUN_SUITE( gap_suite );
    MU_REPORT();
    return MU_EXIT_CODE;
}
