#include "minunit.h"
#include "gac.h"

#undef MINUNIT_EPSILON
#define MINUNIT_EPSILON 1E-7

static gac_filter_noise_t noise_stack;
static gac_filter_noise_t* noise_heap;
static gac_filter_noise_t* noise;

float avg3( float a, float b, float c )
{
    return ( a + b + c ) / 3.0F;
}

void noise_setup()
{
    gac_filter_noise_init( &noise_stack, GAC_FILTER_NOISE_TYPE_AVERAGE, 1 );
    noise = &noise_stack;
}

void noise_teardown()
{
    gac_filter_noise_destroy( noise );
}

MU_TEST( noise_init_heap )
{
    noise_heap = gac_filter_noise_create( GAC_FILTER_NOISE_TYPE_AVERAGE, 1 );
    noise = noise_heap;
    mu_check( noise->window.length == 3 );
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
    MU_RUN_TEST( noise_init_heap );
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
    mu_check( sample == NULL );
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
    mu_check( sample == NULL );

    sample = gac_filter_noise( noise, sample2 );
    mu_check( sample == NULL );
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
    mu_check( sample == NULL );

    sample = gac_filter_noise( noise, sample2 );
    mu_check( sample == NULL );

    sample = gac_filter_noise( noise, sample3 );
    mu_assert_double_eq( avg3( o[0], o2[0], o3[0] ), sample->origin[0] );
    mu_assert_double_eq( avg3( o[1], o2[1], o3[1] ), sample->origin[1] );
    mu_assert_double_eq( avg3( o[2], o2[2], o3[2] ), sample->origin[2] );
    mu_assert_double_eq( avg3( p[0], p2[0], p3[0] ), sample->point[0] );
    mu_assert_double_eq( avg3( p[1], p2[1], p3[1] ), sample->point[1] );
    mu_assert_double_eq( avg3( p[2], p2[2], p3[2] ), sample->point[2] );
    mu_assert_double_eq( timestamp2, sample->timestamp );
    gac_sample_destroy( sample );
}

MU_TEST( noise_4 )
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

    double timestamp4 = 4.12345;
    float o4[3] = { 0.6, 0.7, 0.8 };
    float p4[3] = { 0.9, 0.9, 0.9 };
    gac_sample_t* sample4 = gac_sample_create( &o4, &p4, timestamp4 );

    sample = gac_filter_noise( noise, sample );
    mu_check( sample == NULL );

    sample = gac_filter_noise( noise, sample2 );
    mu_check( sample == NULL );

    sample = gac_filter_noise( noise, sample3 );
    mu_assert_double_eq( avg3( o[0], o2[0], o3[0] ), sample->origin[0] );
    mu_assert_double_eq( avg3( o[1], o2[1], o3[1] ), sample->origin[1] );
    mu_assert_double_eq( avg3( o[2], o2[2], o3[2] ), sample->origin[2] );
    mu_assert_double_eq( avg3( p[0], p2[0], p3[0] ), sample->point[0] );
    mu_assert_double_eq( avg3( p[1], p2[1], p3[1] ), sample->point[1] );
    mu_assert_double_eq( avg3( p[2], p2[2], p3[2] ), sample->point[2] );
    mu_assert_double_eq( timestamp2, sample->timestamp );
    gac_sample_destroy( sample );

    sample = gac_filter_noise( noise, sample4 );
    mu_assert_double_eq( avg3( o4[0], o2[0], o3[0] ), sample->origin[0] );
    mu_assert_double_eq( avg3( o4[1], o2[1], o3[1] ), sample->origin[1] );
    mu_assert_double_eq( avg3( o4[2], o2[2], o3[2] ), sample->origin[2] );
    mu_assert_double_eq( avg3( p4[0], p2[0], p3[0] ), sample->point[0] );
    mu_assert_double_eq( avg3( p4[1], p2[1], p3[1] ), sample->point[1] );
    mu_assert_double_eq( avg3( p4[2], p2[2], p3[2] ), sample->point[2] );
    mu_assert_double_eq( timestamp3, sample->timestamp );
    gac_sample_destroy( sample );
}

void noise_run()
{
    MU_RUN_TEST( noise_1 );
    MU_RUN_TEST( noise_2 );
    MU_RUN_TEST( noise_3 );
    MU_RUN_TEST( noise_4 );
}

MU_TEST_SUITE( noise_suite )
{
    MU_SUITE_CONFIGURE( &noise_setup, &noise_teardown );
    noise_run();
}

int main()
{
    MU_RUN_SUITE( noise_init_suite );
    MU_RUN_SUITE( noise_suite );
    MU_REPORT();
    return MU_EXIT_CODE;
}
