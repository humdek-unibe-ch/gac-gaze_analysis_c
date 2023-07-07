#include "minunit.h"
#include "gac.h"

#undef MINUNIT_EPSILON
#define MINUNIT_EPSILON 1E-7

static gac_t* h;
static gac_t* h_heap;
static gac_t h_stack;
static gac_filter_parameter_t params;

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
}

void h_teardown()
{
    gac_destroy( h );
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

void h_run()
{
    MU_RUN_TEST( h_param );
}

MU_TEST_SUITE( h_default_suite )
{
    MU_SUITE_CONFIGURE( &h_setup_default, &h_teardown );
    h_run();
}

int main()
{
    MU_RUN_SUITE( h_init_suite );
    MU_RUN_SUITE( h_default_suite );
    MU_REPORT();
    return MU_EXIT_CODE;
}
