#include "minunit.h"
#include "gac.h"

#undef MINUNIT_EPSILON
#define MINUNIT_EPSILON 1E-7

#define SAMPLE_COUNT 11

static gac_filter_fixation_t* fixation;
static gac_filter_fixation_t* fixation_heap;
static gac_filter_fixation_t fixation_stack;
static float points[SAMPLE_COUNT][3] =
{
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

void fixation_setup()
{
    gac_filter_fixation_init( &fixation_stack, 0.5, 100 );
    fixation = &fixation_stack;
    idx = 0;
    timestamp = 1000;
}

void fixation_teardown()
{
    gac_filter_fixation_destroy( fixation );
}

MU_TEST( fixation_init_stack )
{
    fixation_setup();
}

MU_TEST( fixation_init_heap )
{
    fixation_heap = gac_filter_fixation_create( 0.5, 100 );
    fixation = fixation_heap;
}

MU_TEST_SUITE( h_init_suite )
{
    MU_SUITE_CONFIGURE( NULL, &fixation_teardown );
    MU_RUN_TEST( fixation_init_stack );
    MU_RUN_TEST( fixation_init_heap );
}

bool add_sample( gac_fixation_t* point )
{
    timestamp += 1000.0 / 60;
    gac_sample_t* sample = gac_sample_create( &screen_point, &origins[idx], &points[idx], timestamp, 0, NULL );
    idx++;

    return gac_filter_fixation( fixation, sample, point );
}

MU_TEST( fixation_0 )
{
    bool res;
    gac_fixation_t point;

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
    mu_check( res == false );
    res = add_sample( &point );
    mu_check( res == false );
    idx++;
    idx++;
    res = add_sample( &point );
    mu_check( res == false );
}

MU_TEST( fixation_1 )
{
    float point_avg[3];
    bool res;
    gac_fixation_t point;

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
    mu_check( res == false );
    res = add_sample( &point );
    mu_check( res == false );
    res = add_sample( &point );
    mu_check( res == false );
    res = add_sample( &point );
    mu_check( res == false );
    res = add_sample( &point );
    mu_check( res == true );
    avg( 2, 9, points, point_avg );
    mu_assert_double_eq( point_avg[0], point.point[0] );
    mu_assert_double_eq( point_avg[1], point.point[1] );
    mu_assert_double_eq( point_avg[2], point.point[2] );
    mu_assert_double_eq( 7 * 1000.0 / 60, point.duration );
    mu_assert_double_eq( 1000 + 3 * 1000.0 / 60, point.timestamp );
}

MU_TEST_SUITE( h_default_suite )
{
    MU_SUITE_CONFIGURE( &fixation_setup, &fixation_teardown );
    MU_RUN_TEST( fixation_0 );
    MU_RUN_TEST( fixation_1 );
}

int main()
{
    MU_RUN_SUITE( h_init_suite );
    MU_RUN_SUITE( h_default_suite );
    MU_REPORT();
    return MU_EXIT_CODE;
}
