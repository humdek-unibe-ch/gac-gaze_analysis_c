#include "gac.h"
#include <csv.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

void csv_cb( void* s, size_t len, void* data )
{
    void*** vals = data;
    ( void )len;
    **vals = strdup( s );
    (*vals)++;
}

bool atob( const char* a )
{
    if( a == NULL || strcmp( a, "True" ) == 0 || strcmp( a, "true" ) == 0 || strcmp( a, "TRUE" ) == 0 )
    {
        return true;
    }

    return false;
}

void compute( int count, void* h, FILE* fp_fixations, FILE* fp_saccades )
{
    int i;
    bool res;
    gac_fixation_t fixation;
    gac_saccade_t saccade;

    for( i = 0; i < count; i++ )
    {
        res = gac_sample_window_fixation_filter( h, &fixation );
        if( res == true )
        {
            fprintf( fp_fixations, "%f, %f, %f, %d, %s, %f, %f, %f, %f, %f, %f\n",
                    fixation.first_sample.timestamp,
                    fixation.first_sample.trial_onset,
                    fixation.first_sample.label_onset,
                    fixation.first_sample.trial_id,
                    fixation.first_sample.label,
                    fixation.screen_point[0],
                    fixation.screen_point[1],
                    fixation.point[0],
                    fixation.point[1],
                    fixation.point[2],
                    fixation.duration );
            gac_fixation_destroy( &fixation );
        }
        res = gac_sample_window_saccade_filter( h, &saccade );
        if( res == true )
        {
            fprintf( fp_saccades, "%f, %f, %f, %d, %s, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f\n",
                    saccade.first_sample.timestamp,
                    saccade.first_sample.trial_onset,
                    saccade.first_sample.label_onset,
                    saccade.first_sample.trial_id,
                    saccade.first_sample.label,
                    saccade.first_sample.screen_point[0],
                    saccade.first_sample.screen_point[1],
                    saccade.first_sample.point[0],
                    saccade.first_sample.point[1],
                    saccade.first_sample.point[2],
                    saccade.last_sample.screen_point[0],
                    saccade.last_sample.screen_point[1],
                    saccade.last_sample.point[0],
                    saccade.last_sample.point[1],
                    saccade.last_sample.point[2],
                    saccade.last_sample.timestamp - saccade.first_sample.timestamp );
            gac_saccade_destroy( &saccade );
        }
    }
    gac_sample_window_cleanup( h );
}

int main(int argc, char* argv[])
{
    gac_t h, h_screen;
    bool first = true;
    int rc, len, count, i;
    gac_filter_parameter_t params;
    char line[10000];
    struct csv_parser p;
    FILE* fp;
    FILE* fp_fixations;
    FILE* fp_saccades;
    FILE* fp_fixations_screen;
    FILE* fp_saccades_screen;

    void* vals[100];
    void* vals_ptr;

    for( i = 0; i < 100; i++ )
    {
        vals[i] = NULL;
    }

    if( argc == 2 )
    {
        fp = fopen( argv[1], "r" );
    }
    else
    {
        fp = fopen( "./sample.csv", "r" );
    }
    fp_fixations = fopen( "./fixations.csv", "w" );
    fp_saccades = fopen( "./saccades.csv", "w" );
    fp_fixations_screen = fopen( "./fixations_screen.csv", "w" );
    fp_saccades_screen = fopen( "./saccades_screen.csv", "w" );

    params.fixation.dispersion_threshold = 0.5;
    params.fixation.duration_threshold = 100;
    params.saccade.velocity_threshold = 20;
    params.noise.mid_idx = 1;
    params.noise.type = GAC_FILTER_NOISE_TYPE_AVERAGE;
    params.gap.max_gap_length = 100;
    params.gap.sample_period = 1000.0/60.0;
    gac_init( &h, &params );
    gac_init( &h_screen, &params );
    gac_set_screen( &h_screen,
      -298.64031982421875, 331.7396545410156, 113.90633392333984,
      298.87738037109375, 331.7396545410156, 113.90633392333984,
      -298.64031982421875, 15.905486106872559, -1.0478993654251099 );
    fprintf( fp_fixations, "timestamp,trial_onset,label_onset,trial_id,"
            "label,sx,sy,px,py,pz,duration\n" );
    fprintf( fp_saccades, "timestamp,trial_onset,label_onset,trial_id,"
            "label,s1x,s1y,p1x,p1y,p1z,s2x,s2y,p2x,p2y,p2z,duration\n" );

    while( fgets( line, 10000, fp ) )
    {
        if( first )
        {
            first = false;
            continue;
        }
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

        if( !atob( vals[11] ) || !atob( vals[12] ) || !atob( vals[13] ) )
        {
            goto next_loop;
        }

        count = gac_sample_window_update_screen( &h,
                atof( vals[5] ), atof( vals[6] ), atof( vals[7] ),
                atof( vals[2] ), atof( vals[3] ), atof( vals[4] ),
                atof( vals[0] ), atof( vals[1] ),
                atof( vals[8] ), atoi( vals[9] ), vals[10] );
        compute( count, &h, fp_fixations, fp_saccades );
        count = gac_sample_window_update( &h_screen,
                atof( vals[5] ), atof( vals[6] ), atof( vals[7] ),
                atof( vals[2] ), atof( vals[3] ), atof( vals[4] ),
                atof( vals[8] ), atoi( vals[9] ), vals[10] );
        compute( count, &h_screen, fp_fixations_screen, fp_saccades_screen );
next_loop:
        for( i = 0; i < 100; i++ )
        {
            free( vals[i] );
        }
    }

    gac_destroy( &h );
    gac_destroy( &h_screen );
    fclose( fp );
    fclose( fp_fixations );
    fclose( fp_saccades );
    fclose( fp_fixations_screen );
    fclose( fp_saccades_screen );

    return 0;
}
