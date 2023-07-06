#include "minunit.h"
#include "gac.h"

static gac_queue_t q_stack;
static gac_queue_t* q_heap;
static gac_queue_t* q;
static unsigned int length = 0;
static unsigned int count = 0;
static double vals[10] = {
    0.01,
    0.02,
    0.03,
    0.04,
    0.05,
    0.06,
    0.07,
    0.08,
    0.09,
    0.10,
};
static unsigned int push_idx = 0;
static unsigned int pop_idx = 0;

extern double minunit_real_timer;
extern double minunit_proc_timer;

extern int minunit_run;
extern int minunit_assert;
extern int minunit_fail;
extern int minunit_status;

void heap_setup()
{
    q_heap = gac_queue_create( 0 );
    q = q_heap;
}

void heap_teardown()
{
    gac_queue_destroy( q_heap, free );
}

void stack_setup()
{
    gac_queue_init( &q_stack, 0 );
    q = &q_stack;
}

void stack_teardown()
{
    gac_queue_destroy( &q_stack, free );
}

MU_TEST( grow )
{
    gac_queue_grow( q, 5 );
    length += 5;
    mu_check( q->length == length );
    mu_check( q->count == count );
}

MU_TEST( push )
{
    double* x = malloc( sizeof( double ) );
    *x = vals[push_idx];
    push_idx++;
    gac_queue_push( q, x );
    count++;
    if( count > length )
    {
        length++;
    }
    mu_check( q->length == length );
    mu_check( q->count == count );
}

MU_TEST( pop )
{
    void* x;
    gac_queue_pop( q, &x, false );
    count--;
    mu_check( q->length == length );
    mu_check( q->count == count );
    mu_check( *( double* )x == vals[pop_idx] );
    pop_idx++;
    free( x );
}

void testcases()
{
    MU_RUN_TEST( push );
    MU_RUN_TEST( pop );
    MU_RUN_TEST( push );
    MU_RUN_TEST( push );
    MU_RUN_TEST( pop );
    MU_RUN_TEST( push );
    MU_RUN_TEST( push );
    MU_RUN_TEST( push );
    MU_RUN_TEST( pop );
    MU_RUN_TEST( pop );
    MU_RUN_TEST( pop );
    MU_RUN_TEST( grow );
    MU_RUN_TEST( push );
    MU_RUN_TEST( push );
    MU_RUN_TEST( push );
    MU_RUN_TEST( pop );
    MU_RUN_TEST( pop );
    MU_RUN_TEST( pop );
}

void report( const char* name )
{
    printf( "%s", name );
    MU_REPORT();
    minunit_real_timer = 0;
    minunit_proc_timer = 0;

    minunit_run = 0;
    minunit_assert = 0;
    minunit_fail = 0;
    minunit_status = 0;
    count = 0;
    length = 0;

    push_idx = 0;
    pop_idx = 0;
}

MU_TEST_SUITE(heap_suite)
{
    heap_setup();
    /* MU_SUITE_CONFIGURE( &heap_setup, &heap_teardown ); */
    testcases();
    report("heap");
    heap_teardown();
}

MU_TEST_SUITE(stack_suite)
{
    stack_setup();
    /* MU_SUITE_CONFIGURE( &heap_setup, &heap_teardown ); */
    testcases();
    report("stack");
    stack_teardown();
}

int main()
{
    MU_RUN_SUITE( heap_suite );
    MU_RUN_SUITE( stack_suite );
    return MU_EXIT_CODE;
}
