#include "minunit.h"
#include "gac.h"

#undef MINUNIT_EPSILON
#define MINUNIT_EPSILON 1E-7

static gac_filter_noise_t noise_stack;
static gac_filter_noise_t* noise_heap;
static gac_filter_noise_t* noise;

void noise_stack_setup()
{
    gac_filter_noise_init( &noise_stack, GAC_FILTER_NOISE_TYPE_AVERAGE, 1 );
    noise = &noise_stack;
}

void noise_heap_setup()
{
    noise_heap = gac_filter_noise_create( GAC_FILTER_NOISE_TYPE_AVERAGE, 1 );
    noise = noise_heap;
}

void noise_teardown()
{
    gac_filter_noise_destroy( noise );
}

MU_TEST( noise_init_1 )
{
    gac_filter_noise_init( &noise_stack, GAC_FILTER_NOISE_TYPE_AVERAGE, 1 );
    noise = &noise_stack;
    mu_check( noise->window.length == 3 );
}

MU_TEST( noise_init_n )
{
    gac_filter_noise_init( &noise_stack, GAC_FILTER_NOISE_TYPE_AVERAGE, 10 );
    noise = &noise_stack;
    mu_check( noise->window.length == 21 );
}

MU_TEST_SUITE( noise_init_suite )
{
    MU_SUITE_CONFIGURE( NULL, &noise_teardown );
    MU_RUN_TEST( noise_init_1 );
    MU_RUN_TEST( noise_init_n );
}

MU_TEST( noise_1 )
{
    double timestamp = 1.12345;
    float o[3] = { 0.1, 0.2, 0.3 };
    float p[3] = { 0.4, 0.5, 0.6 };
    gac_sample_t* sample = gac_sample_create( &o, &p, timestamp );
    sample = gac_filter_noise( noise, sample );
    mu_check( sample->origin[0] == o[0] );
    mu_check( sample->origin[1] == o[1] );
    mu_check( sample->origin[2] == o[2] );
    mu_check( sample->point[0] == p[0] );
    mu_check( sample->point[1] == p[1] );
    mu_check( sample->point[2] == p[2] );
    mu_check( sample->timestamp == timestamp );
    gac_sample_destroy( sample );
}

MU_TEST( noise_2 )
{
    double timestamp = 1.12345;
    float o[3] = { 0.1, 0.2, 0.3 };
    float p[3] = { 0.4, 0.5, 0.6 };
    gac_sample_t* sample = gac_sample_create( &o, &p, timestamp );

    double timestamp2= 2.12345;
    float o2[3] = { 0.2, 0.3, 0.4 };
    float p2[3] = { 0.5, 0.6, 0.7 };
    gac_sample_t* sample2 = gac_sample_create( &o2, &p2, timestamp2 );

    sample = gac_filter_noise( noise, sample );
    mu_check( sample->origin[0] == o[0] );
    mu_check( sample->origin[1] == o[1] );
    mu_check( sample->origin[2] == o[2] );
    mu_check( sample->point[0] == p[0] );
    mu_check( sample->point[1] == p[1] );
    mu_check( sample->point[2] == p[2] );
    mu_check( sample->timestamp == timestamp );
    gac_sample_destroy( sample );

    sample = gac_filter_noise( noise, sample2 );
    mu_check( sample->origin[0] == o2[0] );
    mu_check( sample->origin[1] == o2[1] );
    mu_check( sample->origin[2] == o2[2] );
    mu_check( sample->point[0] == p2[0] );
    mu_check( sample->point[1] == p2[1] );
    mu_check( sample->point[2] == p2[2] );
    mu_check( sample->timestamp == timestamp2 );
    gac_sample_destroy( sample );
}

MU_TEST( noise_3 )
{
    double timestamp = 1.12345;
    float o[3] = { 0.1, 0.2, 0.3 };
    float p[3] = { 0.4, 0.5, 0.6 };
    gac_sample_t* sample = gac_sample_create( &o, &p, timestamp );

    double timestamp2 = 2.12345;
    float o2[3] = { 0.2, 0.3, 0.4 };
    float p2[3] = { 0.5, 0.6, 0.7 };
    gac_sample_t* sample2 = gac_sample_create( &o2, &p2, timestamp2 );

    double timestamp3 = 3.12345;
    float o3[3] = { 0.4, 0.5, 0.6 };
    float p3[3] = { 0.7, 0.8, 0.9 };
    gac_sample_t* sample3 = gac_sample_create( &o3, &p3, timestamp3 );

    sample = gac_filter_noise( noise, sample );
    mu_assert_double_eq( sample->origin[0], o[0] );
    mu_assert_double_eq( sample->origin[1], o[1] );
    mu_assert_double_eq( sample->origin[2], o[2] );
    mu_assert_double_eq( sample->point[0], p[0] );
    mu_assert_double_eq( sample->point[1], p[1] );
    mu_assert_double_eq( sample->point[2], p[2] );
    mu_assert_double_eq( sample->timestamp, timestamp );
    gac_sample_destroy( sample );

    sample = gac_filter_noise( noise, sample2 );
    mu_assert_double_eq( sample->origin[0], o2[0] );
    mu_assert_double_eq( sample->origin[1], o2[1] );
    mu_assert_double_eq( sample->origin[2], o2[2] );
    mu_assert_double_eq( sample->point[0], p2[0] );
    mu_assert_double_eq( sample->point[1], p2[1] );
    mu_assert_double_eq( sample->point[2], p2[2] );
    mu_assert_double_eq( sample->timestamp, timestamp2 );
    gac_sample_destroy( sample );

    sample = gac_filter_noise( noise, sample3 );
    mu_assert_double_eq( sample->origin[0], ( o[0] + o2[0] + o3[0] ) / 3.0F );
    mu_assert_double_eq( sample->origin[1], ( o[1] + o2[1] + o3[1] ) / 3.0F );
    mu_assert_double_eq( sample->origin[2], ( o[2] + o2[2] + o3[2] ) / 3.0F );
    mu_assert_double_eq( sample->point[0], ( p[0] + p2[0] + p3[0] ) / 3.0F );
    mu_assert_double_eq( sample->point[1], ( p[1] + p2[1] + p3[1] ) / 3.0F );
    mu_assert_double_eq( sample->point[2], ( p[2] + p2[2] + p3[2] ) / 3.0F );
    mu_assert_double_eq( sample->timestamp, timestamp2 );
    gac_sample_destroy( sample );
}

void noise_run()
{
    MU_RUN_TEST( noise_1 );
    MU_RUN_TEST( noise_2 );
    MU_RUN_TEST( noise_3 );
}

MU_TEST_SUITE( noise_stack_suite )
{
    MU_SUITE_CONFIGURE( &noise_stack_setup, &noise_teardown );
    noise_run();
}

MU_TEST_SUITE( noise_heap_suite )
{
    MU_SUITE_CONFIGURE( &noise_heap_setup, &noise_teardown );
    noise_run();
}

int main()
{
    MU_RUN_SUITE( noise_init_suite );
    MU_RUN_SUITE( noise_stack_suite );
    MU_RUN_SUITE( noise_heap_suite );
    MU_REPORT();
    return MU_EXIT_CODE;
}
