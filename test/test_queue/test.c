#include "minunit.h"
#include "gac.h"

static gac_queue_t q_stack;
static gac_queue_t* q_heap;
static gac_queue_t* q;
static unsigned int length = 0;
static unsigned int count = 0;
static double vals[16] = {
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
    0.11,
    0.12,
    0.13,
    0.14,
    0.15,
    0.16,
};
static unsigned int push_idx = 0;
static unsigned int pop_idx = 0;

void teardown()
{
    gac_queue_destroy( q );
    count = 0;
    length = 0;

    push_idx = 0;
    pop_idx = 0;
}

void setup()
{
    gac_queue_init( &q_stack, 0 );
    gac_queue_set_rm_handler( &q_stack, free );
    q = &q_stack;
}

void push()
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
    mu_assert_int_eq( length, q->length );
    mu_assert_int_eq( count, q->count );
}

void pop()
{
    void* x;
    gac_queue_pop( q, &x );
    count--;
    mu_assert_int_eq( length, q->length );
    mu_assert_int_eq( count, q->count );
    mu_assert_double_eq( vals[pop_idx], *( double* )x );
    pop_idx++;
    free( x );
}

void rm()
{
    gac_queue_remove( q );
    count--;
    mu_assert_int_eq( length, q->length );
    mu_assert_int_eq( count, q->count );
    pop_idx++;
}

void grow()
{
    gac_queue_grow( q, 5 );
    length += 5;
    mu_assert_int_eq( length, q->length );
    mu_assert_int_eq( count, q->count );
}

MU_TEST( push_1 )
{
    push();
}

MU_TEST( push_1_pop_1 )
{
    push();
    pop();
}

MU_TEST( push_n )
{
    push();
    push();
    push();
    push();
}

MU_TEST( push_n_pop_1 )
{
    push();
    push();
    push();
    push();
    pop();
}

MU_TEST( push_n_pop_n )
{
    push();
    push();
    push();
    push();
    pop();
    pop();
    pop();
    pop();
}

MU_TEST( push_1_rm_1 )
{
    push();
    pop();
}

MU_TEST( push_n_rm_1 )
{
    push();
    push();
    push();
    push();
    rm();
}

MU_TEST( push_n_rm_n )
{
    push();
    push();
    push();
    push();
    rm();
    rm();
    rm();
    rm();
}

MU_TEST( grow_1 )
{
    grow();
}

MU_TEST( grow_1_push_1 )
{
    grow();
    push();
}

MU_TEST( grow_1_push_n )
{
    grow();
    push();
    push();
    push();
    push();
}

MU_TEST( grow_1_push_np )
{
    grow();
    push();
    push();
    push();
    push();
    push();
    push();
}

MU_TEST( grow_n )
{
    grow();
    grow();
}

MU_TEST( grow_n_push_1 )
{
    grow();
    grow();
    push();
}

MU_TEST( grow_n_push_n )
{
    grow();
    grow();
    push();
    push();
    push();
    push();
}

MU_TEST( grow_n_push_np )
{
    grow();
    grow();
    push();
    push();
    push();
    push();
    push();
    push();
    push();
    push();
    push();
    push();
    push();
}

MU_TEST( grow_n_push_np2 )
{
    grow();
    push();
    push();
    push();
    push();
    push();
    push();
    grow();
    push();
    push();
    push();
    push();
    push();
    push();
}

MU_TEST( grow_n_push_np2_pop_np2 )
{
    grow();
    push();
    push();
    push();
    push();
    push();
    push();

    grow();
    push();
    push();
    push();
    push();
    push();
    push();

    pop();
    pop();
    pop();
    pop();
    pop();
    pop();

    pop();
    pop();
    pop();
    pop();
    pop();
    pop();
}

MU_TEST( init_0 )
{
    gac_queue_init( &q_stack, 0 );
    q = &q_stack;
    mu_assert_int_eq( 0, q->length );
    mu_assert_int_eq( 0, q->count );
}

MU_TEST( init_1 )
{
    gac_queue_init( &q_stack, 1 );
    q = &q_stack;
    mu_assert_int_eq( 1, q->length );
    mu_assert_int_eq( 0, q->count );
}

MU_TEST( init_n )
{
    gac_queue_init( &q_stack, 5 );
    q = &q_stack;
    mu_assert_int_eq( 5, q->length );
    mu_assert_int_eq( 0, q->count );
}

MU_TEST( init_heap )
{
    q_heap = gac_queue_create( 0 );
    q = q_heap;
    mu_assert_int_eq( 0, q->length );
    mu_assert_int_eq( 0, q->count );
}

void testcases()
{
    MU_RUN_TEST( push_1 );
    MU_RUN_TEST( push_n );
    MU_RUN_TEST( push_1_pop_1 );
    MU_RUN_TEST( push_n_pop_1 );
    MU_RUN_TEST( push_n_pop_n );
    MU_RUN_TEST( push_1_rm_1 );
    MU_RUN_TEST( push_n_rm_1 );
    MU_RUN_TEST( push_n_rm_n );
    MU_RUN_TEST( grow_1 );
    MU_RUN_TEST( grow_n );
    MU_RUN_TEST( grow_1_push_1 );
    MU_RUN_TEST( grow_1_push_n );
    MU_RUN_TEST( grow_1_push_np );
    MU_RUN_TEST( grow_n_push_1 );
    MU_RUN_TEST( grow_n_push_n );
    MU_RUN_TEST( grow_n_push_np );
    MU_RUN_TEST( grow_n_push_np2 );
    MU_RUN_TEST( grow_n_push_np2_pop_np2 );
}

MU_TEST_SUITE(init_queue_suite)
{
    MU_SUITE_CONFIGURE( NULL, &teardown );
    MU_RUN_TEST( init_heap );
    MU_RUN_TEST( init_0 );
    MU_RUN_TEST( init_1 );
    MU_RUN_TEST( init_n );
}

MU_TEST_SUITE(queue_suite)
{
    MU_SUITE_CONFIGURE( &setup, &teardown );
    testcases();
}

int main()
{
    MU_RUN_SUITE( init_queue_suite );
    MU_RUN_SUITE( queue_suite );
    MU_REPORT();
    return MU_EXIT_CODE;
}
