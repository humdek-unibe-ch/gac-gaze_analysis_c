#include "gac.h"

#define SAMPLE_COUNT 18

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

int main()
{
    int i = 0;
    gac_t h;
    gac_filter_parameter_t params;
    double timestamp = 0;

    gac_fixation_t fixation;
    gac_saccade_t saccade;
    bool res;

    params.fixation.dispersion_threshold = 0.5;
    params.fixation.duration_threshold = 100;
    params.saccade.velocity_threshold = 25;
    params.noise.mid_idx = 0; // noise filter disabled
    params.noise.type = GAC_FILTER_NOISE_TYPE_AVERAGE;
    params.gap.max_gap_length = 0; // gap filter disabled
    params.gap.sample_period = 1000.0/60.0;
    gac_init( &h, &params );

    for( i = 0; i < SAMPLE_COUNT; i++ )
    {
        timestamp += params.gap.sample_period;
        gac_sample_window_update( &h, origins[i][0], origins[i][1], origins[i][2],
                points[i][0], points[i][1], points[i][2], timestamp );
        res = gac_sample_window_fixation_filter( &h, &fixation );
        if( res == true )
        {
            printf( "%f fixation: [%f, %f, %f], %f\n", fixation.timestamp,
                    fixation.point[0], fixation.point[1], fixation.point[2],
                    fixation.duration );
        }
        res = gac_sample_window_saccade_filter( &h, &saccade );
        if( res == true )
        {
            printf( "%f saccade: [%f, %f, %f] -> [%f, %f, %f], %f\n",
                    saccade.timestamp,
                    saccade.point_start[0], saccade.point_start[1], saccade.point_start[2],
                    saccade.point_dest[0], saccade.point_dest[1], saccade.point_dest[2],
                    saccade.duration );
        }
    }

    gac_destroy( &h );

    return 0;
}
