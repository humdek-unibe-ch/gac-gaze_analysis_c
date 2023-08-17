#include "minunit.h"
#include "gac.h"

#undef MINUNIT_EPSILON
#define MINUNIT_EPSILON 1E-7

#define SAMPLE_COUNT 14

static gac_filter_saccade_t* saccade;
static gac_filter_saccade_t* saccade_heap;
static gac_filter_saccade_t saccade_stack;
static float points[SAMPLE_COUNT][3] =
{
    { 300, 301, 500 },
    { 301, 300, 500 },
    { 300, 299, 500 },
    { 300, 300, 500 },
    { 400, 400, 500 },
    { 500, 500, 500 },
    { 501, 499, 499 },
    { 501, 500, 500 },
    { 499, 499, 500 },
    { 499, 499, 500 },
    { 500, 500, 499 },
    { 500, 500, 499 },
    { 501, 499, 500 },
    { 600, 600, 500 }
};

static float origins[SAMPLE_COUNT][3] =
{
    { 495, 505, 10 },
    { 495, 505, 10 },
    { 495, 505, 10 },
    { 495, 505, 10 },
    { 496, 504, 8 },
    { 500, 500, 0 },
    { 501, 501, 0 },
    { 503, 498, 0 },
    { 500, 500, 0 },
    { 503, 487, 3 },
    { 501, 499, 2 },
    { 501, 499, 2 },
    { 502, 500, 1 },
    { 505, 504, 0 }
};

static float screen_point[2] = { 0, 0 };

void avg( int start, int stop, float values[SAMPLE_COUNT][3], float avg[3] )
{
    int i, count = 0;
    avg[0] = 0;
    avg[1] = 0;
    avg[2] = 0;

    for( i = start; i <= stop; i++ )
    {
        avg[0] += values[i][0];
        avg[1] += values[i][1];
        avg[2] += values[i][2];
        count++;
    }

    avg[0] /= count;
    avg[1] /= count;
    avg[2] /= count;
}

static int idx = 0;
static double timestamp = 1000;

void saccade_setup()
{
    gac_filter_saccade_init( &saccade_stack, 20 );
    saccade = &saccade_stack;
    idx = 0;
    timestamp = 1000;
}

void saccade_teardown()
{
    gac_filter_saccade_destroy( saccade );
}

MU_TEST( saccade_init_stack )
{
    saccade_setup();
}

MU_TEST( saccade_init_heap )
{
    saccade_heap = gac_filter_saccade_create( 20 );
    saccade = saccade_heap;
}

MU_TEST_SUITE( h_init_suite )
{
    MU_SUITE_CONFIGURE( NULL, &saccade_teardown );
    MU_RUN_TEST( saccade_init_stack );
    MU_RUN_TEST( saccade_init_heap );
}

bool add_sample( gac_saccade_t* point )
{
    timestamp += 1000.0 / 60;
    gac_sample_t* sample = gac_sample_create( &screen_point, &origins[idx], &points[idx], timestamp, 0, NULL );
    idx++;

    return gac_filter_saccade( saccade, sample, point );
}

MU_TEST( saccade_0 )
{
    bool res;
    gac_saccade_t point;

    res = add_sample( &point );
    mu_check( res == false );
    res = add_sample( &point );
    mu_check( res == false );
    res = add_sample( &point );
    mu_check( res == false );
    res = add_sample( &point );
    mu_check( res == false );
}

MU_TEST( saccade_1 )
{
    bool res;
    gac_saccade_t point;

    res = add_sample( &point );
    mu_check( res == false );
    res = add_sample( &point );
    mu_check( res == false );
    res = add_sample( &point );
    mu_check( res == false );
    res = add_sample( &point );
    mu_check( res == false );
    res = add_sample( &point );
    mu_check( res == false );
    res = add_sample( &point );
    mu_check( res == false );
    res = add_sample( &point );
    mu_check( res == true );
    mu_assert_double_eq( points[3][0], point.first_sample->point[0] );
    mu_assert_double_eq( points[3][1], point.first_sample->point[1] );
    mu_assert_double_eq( points[3][2], point.first_sample->point[2] );
    mu_assert_double_eq( points[5][0], point.last_sample->point[0] );
    mu_assert_double_eq( points[5][1], point.last_sample->point[1] );
    mu_assert_double_eq( points[5][2], point.last_sample->point[2] );
    mu_assert_double_eq( 2 * 1000.0 / 60, point.last_sample->timestamp - point.first_sample->timestamp );
    mu_assert_double_eq( 1000 + 4 * 1000.0 / 60, point.first_sample->timestamp );
}

MU_TEST_SUITE( h_default_suite )
{
    MU_SUITE_CONFIGURE( &saccade_setup, &saccade_teardown );
    MU_RUN_TEST( saccade_0 );
    MU_RUN_TEST( saccade_1 );
}

int main()
{
    MU_RUN_SUITE( h_init_suite );
    MU_RUN_SUITE( h_default_suite );
    MU_REPORT();
    return MU_EXIT_CODE;
}
