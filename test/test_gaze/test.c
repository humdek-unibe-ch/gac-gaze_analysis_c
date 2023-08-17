#include "minunit.h"
#include "gac.h"

#undef MINUNIT_EPSILON
#define MINUNIT_EPSILON 1E-7

#define SAMPLE_COUNT 18

static gac_t* h;
static gac_t* h_heap;
static gac_t h_stack;
static gac_filter_parameter_t params;

static float points[SAMPLE_COUNT][3] =
{
    { 300, 301, 500 },
    { 301, 300, 500 },
    { 300, 299, 500 },
    { 300, 300, 500 }, // s1 start
    { 400, 400, 500 },
    { 500, 500, 500 }, // s1 stop, f1 start
    { 501, 499, 499 },
    { 501, 500, 500 },
    { 499, 499, 500 },
    { 499, 499, 500 },
    { 499, 499, 500 },
    { 499, 499, 500 },
    { 499, 499, 500 },
    { 500, 500, 499 },
    { 500, 500, 499 },
    { 501, 499, 500 }, // f1 stop, s2 start
    { 600, 600, 500 }, // s2 stop
    { 600, 601, 500 }
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
    { 503, 499, 1 },
    { 503, 499, 1 },
    { 503, 499, 2 },
    { 503, 499, 3 },
    { 501, 499, 2 },
    { 501, 499, 2 },
    { 502, 500, 1 },
    { 505, 504, 0 },
    { 505, 504, 0 }
};

static int idx = 0;
static double timestamp = 1000;

void h_setup_default()
{
    gac_init( &h_stack, NULL );
    params.fixation.dispersion_threshold = 0.5;
    params.fixation.duration_threshold = 100;
    params.saccade.velocity_threshold = 20;
    params.noise.mid_idx = 1;
    params.noise.type = GAC_FILTER_NOISE_TYPE_AVERAGE;
    params.gap.max_gap_length = 50;
    params.gap.sample_period = 1000.0/60.0;
    h = &h_stack;
    idx = 0;
    timestamp = 1000;
}

void h_setup_no_filter()
{
    params.fixation.dispersion_threshold = 0.5;
    params.fixation.duration_threshold = 100;
    params.saccade.velocity_threshold = 25;
    params.noise.mid_idx = 0;
    params.noise.type = GAC_FILTER_NOISE_TYPE_AVERAGE;
    params.gap.max_gap_length = 0;
    params.gap.sample_period = 1000.0/60.0;
    gac_init( &h_stack, &params );
    h = &h_stack;
    idx = 0;
    timestamp = 1000;
}

void h_teardown()
{
    gac_destroy( h );
}

void add_sample()
{
    timestamp += 1000.0 / 60;
    gac_sample_window_update( h, origins[idx][0], origins[idx][1],
            origins[idx][2], points[idx][0], points[idx][1], points[idx][2],
            timestamp, 0, NULL );
    idx++;
}

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

MU_TEST( h_init_stack )
{
    h_setup_default();
}

MU_TEST( h_init_heap )
{
    h_heap = gac_create( NULL );
    h = h_heap;
}

MU_TEST_SUITE( h_init_suite )
{
    MU_SUITE_CONFIGURE( NULL, &h_teardown );
    MU_RUN_TEST( h_init_stack );
    MU_RUN_TEST( h_init_heap );
}

MU_TEST( h_param )
{
    gac_filter_parameter_t p;
    gac_get_filter_parameter( h, &p );

    mu_assert_double_eq( params.fixation.dispersion_threshold, p.fixation.dispersion_threshold );
    mu_assert_int_eq( params.fixation.duration_threshold, p.fixation.duration_threshold );
    mu_assert_int_eq( params.saccade.velocity_threshold, p.saccade.velocity_threshold );
    mu_assert_int_eq( params.noise.mid_idx, p.noise.mid_idx );
    mu_assert_int_eq( params.noise.type, p.noise.type );
    mu_assert_int_eq( params.gap.max_gap_length, p.gap.max_gap_length );
    mu_assert_double_eq( params.gap.sample_period, p.gap.sample_period );
}

MU_TEST( h_filter )
{
    gac_fixation_t fixation;
    gac_saccade_t saccade;
    bool res;
    float point_avg[3];

    add_sample();
    res = gac_sample_window_fixation_filter( h, &fixation );
    mu_check( h->fixation.is_collecting == false );
    mu_check( res == false );
    res = gac_sample_window_saccade_filter( h, &saccade );
    mu_check( h->saccade.is_collecting == false );
    mu_check( res == false );

    add_sample();
    res = gac_sample_window_fixation_filter( h, &fixation );
    mu_check( h->fixation.is_collecting == false );
    mu_check( res == false );
    res = gac_sample_window_saccade_filter( h, &saccade );
    mu_check( h->saccade.is_collecting == false );
    mu_check( res == false );

    add_sample();
    res = gac_sample_window_fixation_filter( h, &fixation );
    mu_check( h->fixation.is_collecting == false );
    mu_check( res == false );
    res = gac_sample_window_saccade_filter( h, &saccade );
    mu_check( h->saccade.is_collecting == false );
    mu_check( res == false );

    // s1 start
    add_sample();
    res = gac_sample_window_fixation_filter( h, &fixation );
    mu_check( h->fixation.is_collecting == false );
    mu_check( res == false );
    res = gac_sample_window_saccade_filter( h, &saccade );
    mu_check( h->saccade.is_collecting == false );
    mu_check( res == false );

    // s1 start detected
    add_sample();
    res = gac_sample_window_fixation_filter( h, &fixation );
    mu_check( h->fixation.is_collecting == false );
    mu_check( res == false );
    res = gac_sample_window_saccade_filter( h, &saccade );
    mu_check( h->saccade.is_collecting == true );
    mu_check( res == false );

    // s1 stop
    // f1 start
    add_sample();
    res = gac_sample_window_fixation_filter( h, &fixation );
    mu_check( h->fixation.is_collecting == false );
    mu_check( res == false );
    res = gac_sample_window_saccade_filter( h, &saccade );
    mu_check( h->saccade.is_collecting == true );
    mu_check( res == false );

    // s1 stop detected
    add_sample();
    res = gac_sample_window_fixation_filter( h, &fixation );
    mu_check( h->fixation.is_collecting == false );
    mu_check( res == false );
    res = gac_sample_window_saccade_filter( h, &saccade );
    mu_check( h->saccade.is_collecting == false );
    mu_check( res == true );
    mu_assert_double_eq( points[3][0], saccade.first_sample.point[0] );
    mu_assert_double_eq( points[3][1], saccade.first_sample.point[1] );
    mu_assert_double_eq( points[3][2], saccade.first_sample.point[2] );
    mu_assert_double_eq( points[5][0], saccade.last_sample.point[0] );
    mu_assert_double_eq( points[5][1], saccade.last_sample.point[1] );
    mu_assert_double_eq( points[5][2], saccade.last_sample.point[2] );
    mu_assert_double_eq( 2 * 1000.0 / 60, saccade.last_sample.timestamp - saccade.first_sample.timestamp );
    mu_assert_double_eq( 1000 + 4 * 1000.0 / 60, saccade.first_sample.timestamp );

    add_sample();
    res = gac_sample_window_fixation_filter( h, &fixation );
    mu_check( h->fixation.is_collecting == false );
    mu_check( res == false );
    res = gac_sample_window_saccade_filter( h, &saccade );
    mu_check( h->saccade.is_collecting == false );
    mu_check( res == false );

    add_sample();
    res = gac_sample_window_fixation_filter( h, &fixation );
    mu_check( h->fixation.is_collecting == false );
    mu_check( res == false );
    res = gac_sample_window_saccade_filter( h, &saccade );
    mu_check( h->saccade.is_collecting == false );
    mu_check( res == false );

    add_sample();
    res = gac_sample_window_fixation_filter( h, &fixation );
    mu_check( h->fixation.is_collecting == false );
    mu_check( res == false );
    res = gac_sample_window_saccade_filter( h, &saccade );
    mu_check( h->saccade.is_collecting == false );
    mu_check( res == false );

    add_sample();
    res = gac_sample_window_fixation_filter( h, &fixation );
    mu_check( h->fixation.is_collecting == false );
    mu_check( res == false );
    res = gac_sample_window_saccade_filter( h, &saccade );
    mu_check( h->saccade.is_collecting == false );
    mu_check( res == false );

    // f1 start detected
    add_sample();
    res = gac_sample_window_fixation_filter( h, &fixation );
    mu_check( h->fixation.is_collecting == true );
    mu_check( res == false );
    res = gac_sample_window_saccade_filter( h, &saccade );
    mu_check( h->saccade.is_collecting == false );
    mu_check( res == false );

    add_sample();
    res = gac_sample_window_fixation_filter( h, &fixation );
    mu_check( h->fixation.is_collecting == true );
    mu_check( res == false );
    res = gac_sample_window_saccade_filter( h, &saccade );
    mu_check( h->saccade.is_collecting == false );
    mu_check( res == false );

    add_sample();
    res = gac_sample_window_fixation_filter( h, &fixation );
    mu_check( res == false );
    res = gac_sample_window_saccade_filter( h, &saccade );
    mu_check( h->saccade.is_collecting == false );
    mu_check( res == false );

    add_sample();
    res = gac_sample_window_fixation_filter( h, &fixation );
    mu_check( h->fixation.is_collecting == true );
    mu_check( res == false );
    res = gac_sample_window_saccade_filter( h, &saccade );
    mu_check( h->saccade.is_collecting == false );
    mu_check( res == false );

    add_sample();
    res = gac_sample_window_fixation_filter( h, &fixation );
    mu_check( h->fixation.is_collecting == true );
    mu_check( res == false );
    res = gac_sample_window_saccade_filter( h, &saccade );
    mu_check( h->saccade.is_collecting == false );
    mu_check( res == false );

    add_sample();
    res = gac_sample_window_fixation_filter( h, &fixation );
    mu_check( h->fixation.is_collecting == false );
    mu_check( res == true );
    avg( 5, 15, points, point_avg );
    mu_assert_double_eq( point_avg[0], fixation.point[0] );
    mu_assert_double_eq( point_avg[1], fixation.point[1] );
    mu_assert_double_eq( point_avg[2], fixation.point[2] );
    mu_assert_double_eq( 10 * 1000.0 / 60, fixation.duration );
    mu_assert_double_eq( 1000 + 6 * 1000.0 / 60, fixation.first_sample.timestamp );
    res = gac_sample_window_saccade_filter( h, &saccade );
    mu_check( h->saccade.is_collecting == true );
    mu_check( res == false );

    add_sample();
    res = gac_sample_window_fixation_filter( h, &fixation );
    mu_check( h->fixation.is_collecting == false );
    mu_check( res == false );
    res = gac_sample_window_saccade_filter( h, &saccade );
    mu_check( h->saccade.is_collecting == false );
    mu_check( res == true );
    mu_assert_double_eq( points[15][0], saccade.first_sample.point[0] );
    mu_assert_double_eq( points[15][1], saccade.first_sample.point[1] );
    mu_assert_double_eq( points[15][2], saccade.first_sample.point[2] );
    mu_assert_double_eq( points[16][0], saccade.last_sample.point[0] );
    mu_assert_double_eq( points[16][1], saccade.last_sample.point[1] );
    mu_assert_double_eq( points[16][2], saccade.last_sample.point[2] );
    mu_assert_double_eq( 1000.0 / 60, saccade.last_sample.timestamp - saccade.first_sample.timestamp );
    mu_assert_double_eq( 1000 + 16 * 1000.0 / 60, saccade.first_sample.timestamp );
}

MU_TEST_SUITE( h_default_suite )
{
    MU_SUITE_CONFIGURE( &h_setup_default, &h_teardown );
    MU_RUN_TEST( h_param );
}

MU_TEST_SUITE( h_no_filter_suite )
{
    MU_SUITE_CONFIGURE( &h_setup_no_filter, &h_teardown );
    MU_RUN_TEST( h_filter );
}

int main()
{
    MU_RUN_SUITE( h_init_suite );
    MU_RUN_SUITE( h_default_suite );
    MU_RUN_SUITE( h_no_filter_suite );
    MU_REPORT();
    return MU_EXIT_CODE;
}
