#include "gac.h"
#include <csv.h>
#include <stdio.h>
#include <string.h>

void csv_cb( void* s, size_t len, void* data )
{
    float** vals = data;
    ( void )len;
    **vals = atof( s );
    (*vals)++;
}

int main(int argc, char* argv[])
{
    gac_t h;
    int rc, len, count, i, id = 0;
    gac_filter_parameter_t params;
    char line[1000];
    struct csv_parser p;
    FILE* fp;
    char id_str[16];

    gac_fixation_t fixation;
    gac_saccade_t saccade;
    bool res;
    float vals[7];
    float* vals_ptr = vals;

    if( argc == 2 )
    {
        fp = fopen( argv[1], "r" );
    }
    else
    {
        fp = fopen( "./sample.csv", "r" );
    }

    params.fixation.dispersion_threshold = 0.5;
    params.fixation.duration_threshold = 100;
    params.saccade.velocity_threshold = 25;
    params.noise.mid_idx = 0; // noise filter disabled
    params.noise.type = GAC_FILTER_NOISE_TYPE_AVERAGE;
    params.gap.max_gap_length = 0; // gap filter disabled
    params.gap.sample_period = 1000.0/60.0;
    gac_init( &h, &params );

    while( fgets( line, 1000, fp ) )
    {
        csv_init( &p, CSV_APPEND_NULL );
        csv_set_delim( &p, ',' );
        len = strlen( line );
        vals_ptr = vals;
        rc = csv_parse( &p, line, len, csv_cb, NULL, &vals_ptr );
        if( rc < len )
        {
            printf( "failed to parse CSV line: %s, ignoring\n",
                    csv_strerror( csv_error( &p ) ) );
        }
        csv_fini( &p, csv_cb, NULL, &vals_ptr );
        csv_free( &p );

        sprintf( id_str, "%d", id );
        count = gac_sample_window_update( &h, vals[3], vals[4], vals[5],
                vals[0], vals[1], vals[2], vals[6], id, id_str );
        for( i = 0; i < count; i++ )
        {
            res = gac_sample_window_fixation_filter( &h, &fixation );
            if( res == true )
            {
                printf( "%f fixation(%s): [%f, %f, %f], %f\n", fixation.timestamp, fixation.label,
                        fixation.point[0], fixation.point[1], fixation.point[2],
                        fixation.duration );
                gac_fixation_destroy( &fixation );
            }
            res = gac_sample_window_saccade_filter( &h, &saccade );
            if( res == true )
            {
                printf( "%f saccade(%s): [%f, %f, %f] -> [%f, %f, %f], %f\n",
                        saccade.timestamp, saccade.label,
                        saccade.point_start[0], saccade.point_start[1], saccade.point_start[2],
                        saccade.point_dest[0], saccade.point_dest[1], saccade.point_dest[2],
                        saccade.duration );
                gac_saccade_destroy( &saccade );
            }
        }
        gac_sample_window_cleanup( &h );
        id++;
    }

    gac_destroy( &h );
    fclose( fp );

    return 0;
}
