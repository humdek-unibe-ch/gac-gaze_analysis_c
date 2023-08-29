/**
 * @author  Simon Maurer
 * @license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Example of how to use the gaze analysis library.
 */

#include "gac.h"
#include <csv.h>
#include <string.h>

/**
 * Callback to extract a value form the csv file.
 *
 * @param s
 *  The value as it was extrachted from the file.
 * @param len
 *  The number of bytes extracted.
 * @param data
 *  The context passed to the call back function.
 */
void csv_cb( void* s, size_t len, void* data )
{
    void*** vals = data;
    ( void )len;
    **vals = strdup( s );
    (*vals)++;
}

/**
 * Helper function to convert a boolean string to a boolean value
 *
 * @param a
 *  The boolean string.
 * @return
 *  The boolean value. If string did not match a boolean false is returned.
 */
bool atob( const char* a )
{
    if( a == NULL || strcmp( a, "True" ) == 0 || strcmp( a, "true" ) == 0
            || strcmp( a, "TRUE" ) == 0 )
    {
        return true;
    }

    return false;
}

/**
 * Helper function to perfomr the analysis on the latest samples.
 *
 * @param count
 *  The number of new samples to process
 * @param h
 *  A pointer to the gaze analysis handler
 * @param fp_fixations
 *  A pointer to a file handler for the fixation output.
 * @param fp_saccades
 *  A pointer to a file handler for the saccade output.
 */
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
    const char* fixation_header;
    const char* saccade_header;
    struct csv_parser p;
    FILE* fp;
    FILE* fp_fixations;
    FILE* fp_saccades;
    FILE* fp_fixations_screen;
    FILE* fp_saccades_screen;

    void* vals[100];
    void* vals_ptr;

    printf( "using libgac version %s\n", gac_version() );

    // general initialisation
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

    // set filter parameters
    params.fixation.dispersion_threshold = 0.5;
    params.fixation.duration_threshold = 100;
    params.saccade.velocity_threshold = 20;
    params.noise.mid_idx = 1;
    params.noise.type = GAC_FILTER_NOISE_TYPE_AVERAGE;
    params.gap.max_gap_length = 100;
    params.gap.sample_period = 1000.0/60.0;

    // initialize gaze analysis handler which reads 2d points from sample file
    gac_init( &h, &params );

    // initialize gaze analysis handler which computes 2d points based on
    // screen coordinates
    gac_init( &h_screen, &params );
    gac_set_screen( &h_screen,
      -298.64031982421875, 331.7396545410156, 113.90633392333984,
      298.87738037109375, 331.7396545410156, 113.90633392333984,
      -298.64031982421875, 15.905486106872559, -1.0478993654251099 );

    // init headers for csv files
    fixation_header = "timestamp,trial_onset,label_onset,trial_id,"
            "label,sx,sy,px,py,pz,duration";
    saccade_header = "timestamp,trial_onset,label_onset,trial_id,"
            "label,s1x,s1y,p1x,p1y,p1z,s2x,s2y,p2x,p2y,p2z,duration";
    fprintf( fp_fixations, "%s\n", fixation_header );
    fprintf( fp_fixations_screen, "%s\n", fixation_header );
    fprintf( fp_saccades, "%s\n", saccade_header );
    fprintf( fp_saccades_screen, "%s\n", saccade_header );

    // read sample csv file
    while( fgets( line, 10000, fp ) )
    {
        // skip the header of the sample file
        if( first )
        {
            first = false;
            continue;
        }

        // parse the csv line
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

        // skip to next line if any value is not valid
        if( !atob( vals[11] ) || !atob( vals[12] ) || !atob( vals[13] ) )
        {
            goto next_loop;
        }

        // perform analysis by propagating 2d data from the sample file
        count = gac_sample_window_update_screen( &h,
                atof( vals[5] ), atof( vals[6] ), atof( vals[7] ),
                atof( vals[2] ), atof( vals[3] ), atof( vals[4] ),
                atof( vals[0] ), atof( vals[1] ),
                atof( vals[8] ), atoi( vals[9] ), vals[10] );
        compute( count, &h, fp_fixations, fp_saccades );

        // perform analysis by computing 2d data from screen coordinates
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

    // cleanup
    gac_destroy( &h );
    gac_destroy( &h_screen );
    fclose( fp );
    fclose( fp_fixations );
    fclose( fp_saccades );
    fclose( fp_fixations_screen );
    fclose( fp_saccades_screen );

    return 0;
}
