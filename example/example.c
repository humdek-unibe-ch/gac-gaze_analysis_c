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
#include "gac_aoi_collection.h"
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
 * Write AOI data to an output file.
 *
 * @param result
 *  The AOI analysis result to write.
 * @param fp_aoi
 *  The ouput file pointer.
 */
void write_aoi( gac_aoi_collection_analysis_result_t* result, FILE* fp_aoi )
{
    uint32_t i;
    gac_aoi_analysis_t* analysis;
    double trial_timestamp;
    double label_timestamp;
    double first_saccade_start_onset;
    double first_saccade_end_onset;
    double first_fixation_onset;
    double label_onset;

    for( i = 0; i < result->aois.count; i++ )
    {
        analysis = &result->aois.items[i].analysis;
        trial_timestamp = gac_sample_get_trial_timestamp(
                &analysis->first_fixation.first_sample );
        label_timestamp = gac_sample_get_label_timestamp(
                &analysis->first_fixation.first_sample );
        first_saccade_start_onset = gac_sample_get_onset(
                &analysis->first_saccade.first_sample, trial_timestamp );
        first_saccade_end_onset = gac_sample_get_onset(
                &analysis->first_saccade.last_sample, trial_timestamp );
        first_fixation_onset = gac_sample_get_onset(
                &analysis->first_fixation.first_sample, trial_timestamp );
        label_onset = label_timestamp - trial_timestamp;
        if( label_onset < 0 )
        {
            label_onset = 0;
        }
        if( analysis->fixation_count != 0 )
        {
            fprintf( fp_aoi, "%d,%f,%f,%f,%f,%f,%d,%f,%f,%f,%d,%f,%d,%s,%f\n",
                    result->trial_id,
                    trial_timestamp,
                    analysis->dwell_time,
                    analysis->dwell_time_relative,
                    analysis->first_fixation.duration,
                    first_fixation_onset,
                    analysis->aoi_visited_before_count,
                    first_saccade_start_onset,
                    first_saccade_end_onset,
                    first_saccade_start_onset - label_onset,
                    analysis->enter_saccade_count,
                    analysis->fixation_count_relative,
                    analysis->fixation_count,
                    result->aois.items[i].label,
                    label_onset
                );
        }
    }
}

/**
 * Write fixation data to an output file.
 *
 * @param fixation
 *  The fixation to write.
 * @param fp_fixations
 *  The ouput file pointer.
 */
void write_fixation( gac_fixation_t* fixation, FILE* fp_fixations )
{
    fprintf( fp_fixations, "%f,%f,%f,%d,%s,%f,%f,%f,%f,%f,%f\n",
            fixation->first_sample.timestamp,
            fixation->first_sample.trial_onset,
            fixation->first_sample.label_onset,
            fixation->first_sample.trial_id,
            fixation->first_sample.label,
            fixation->screen_point[0],
            fixation->screen_point[1],
            fixation->point[0],
            fixation->point[1],
            fixation->point[2],
            fixation->duration );
}

/**
 * Write saccade data to an output file.
 *
 * @param saccade
 *  The saccade to write.
 * @param fp_saccade
 *  The ouput file pointer.
 */
void write_saccade( gac_saccade_t* saccade, FILE* fp_saccades )
{
    fprintf( fp_saccades, "%f,%f,%f,%d,%s,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",
            saccade->first_sample.timestamp,
            saccade->first_sample.trial_onset,
            saccade->first_sample.label_onset,
            saccade->first_sample.trial_id,
            saccade->first_sample.label,
            saccade->first_sample.screen_point[0],
            saccade->first_sample.screen_point[1],
            saccade->first_sample.point[0],
            saccade->first_sample.point[1],
            saccade->first_sample.point[2],
            saccade->last_sample.screen_point[0],
            saccade->last_sample.screen_point[1],
            saccade->last_sample.point[0],
            saccade->last_sample.point[1],
            saccade->last_sample.point[2],
            saccade->last_sample.timestamp - saccade->first_sample.timestamp );
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
 * @param fp_aoi
 *  A pointer to a file handler for the aoi output.
 */
void compute( uint32_t count, gac_t* h, FILE* fp_fixations, FILE* fp_saccades,
        FILE* fp_aoi )
{
    uint32_t i;
    bool res;
    gac_fixation_t fixation;
    gac_saccade_t saccade;
    gac_aoi_collection_analysis_result_t analysis;

    for( i = 0; i < count; i++ )
    {
        res = gac_sample_window_saccade_filter( h, &saccade );
        if( res == true )
        {
            write_saccade( &saccade, fp_saccades );
            gac_aoi_collection_analyse_saccade( &h->aoic, &saccade );
            gac_saccade_destroy( &saccade );
        }
        res = gac_sample_window_fixation_filter( h, &fixation );
        if( res == true )
        {
            write_fixation( &fixation, fp_fixations );
            res = gac_aoi_collection_analyse_fixation( &h->aoic, &fixation,
                    &analysis );
            if( res == true )
            {
                write_aoi( &analysis, fp_aoi );
            }
            gac_fixation_destroy( &fixation );
        }
    }
    gac_sample_window_cleanup( h );
}

/**
 * The main application entry.
 *
 * @param argc
 *  The number of arguments passed to the application.
 * @param argv
 *  The argument list passed to the application.
 * @return
 *  The application exit code.
 */
int main( int argc, char* argv[] )
{
    gac_t h, h_screen;
    bool first = true;
    int rc, len, i;
    uint32_t count;
    gac_filter_parameter_t params;
    char line[10000];
    const char* fixation_header;
    const char* saccade_header;
    const char* aoi_header;
    struct csv_parser p;
    FILE* fp;
    FILE* fp_fixations;
    FILE* fp_saccades;
    FILE* fp_aoi;
    FILE* fp_fixations_screen;
    FILE* fp_saccades_screen;
    FILE* fp_aoi_screen;
    gac_aoi_t aoi;
    bool res;
    gac_aoi_collection_analysis_result_t analysis;

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
    fp_aoi = fopen( "./aoi.csv", "w" );
    fp_fixations_screen = fopen( "./fixations_screen.csv", "w" );
    fp_saccades_screen = fopen( "./saccades_screen.csv", "w" );
    fp_aoi_screen = fopen( "./aoi_screen.csv", "w" );

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
    aoi_header = "trial_id,trial_timestamp,dwell_time,dwell_time_rel,"
            "first_fixation_duration,first_fixation_onset,"
            "first_fixation_visited_ia_count,first_saccade_start_onset,"
            "first_saccade_end_onset,first_saccade_latency,enter_saccade_count,"
            "fixation_count_rel,fixation_count,label,label_onset";
    fprintf( fp_fixations, "%s\n", fixation_header );
    fprintf( fp_fixations_screen, "%s\n", fixation_header );
    fprintf( fp_saccades, "%s\n", saccade_header );
    fprintf( fp_saccades_screen, "%s\n", saccade_header );
    fprintf( fp_aoi, "%s\n", aoi_header );
    fprintf( fp_aoi_screen, "%s\n", aoi_header );

    // init aoi
    gac_aoi_init( &aoi, "aoi0" );
    gac_aoi_add_point( &aoi, 0.5, 0.4 );
    gac_aoi_add_point( &aoi, 0.5, 0.3 );
    gac_aoi_add_point( &aoi, 0.6, 0.2 );
    gac_aoi_add_point( &aoi, 0.7, 0.2 );
    gac_aoi_add_point( &aoi, 0.8, 0.3 );
    gac_aoi_add_point( &aoi, 0.8, 0.4 );
    gac_aoi_add_point( &aoi, 0.7, 0.5 );
    gac_aoi_add_point( &aoi, 0.6, 0.5 );
    gac_aoi_collection_add( &h.aoic, &aoi );
    gac_aoi_collection_add( &h_screen.aoic, &aoi );
    gac_aoi_init( &aoi, "aoi1" );
    gac_aoi_set_resolution( &aoi, 2560, 1440 );
    gac_aoi_add_rect( &aoi, 0.3, 0.45, 0.1, 0.1 );
    gac_add_aoi( &h, &aoi );
    gac_add_aoi( &h_screen, &aoi );
    gac_aoi_init( &aoi, "aoi2" );
    gac_aoi_set_resolution( &aoi, 2560, 1440 );
    gac_aoi_add_rect( &aoi, 0.5, 0.75, 0.2, 0.2 );
    gac_add_aoi( &h, &aoi );
    gac_add_aoi( &h_screen, &aoi );
    gac_aoi_init( &aoi, "aoi3" );
    gac_aoi_set_resolution( &aoi, 2560, 1440 );
    gac_aoi_add_rect( &aoi, 0.1, 0.3, 0.2, 0.1 );
    gac_add_aoi( &h, &aoi );
    gac_add_aoi( &h_screen, &aoi );
    /* printf( "aoi\n origin: [%f, %f]\n avg_edge_len: %f\n bounding_box: [%f, %f, %f, %f]\n points: ", */
    /*         aoi.ray_origin[0], aoi.ray_origin[1], aoi.avg_edge_len, */
    /*         aoi.bounding_box.x_min, aoi.bounding_box.x_max, */
    /*         aoi.bounding_box.y_min, aoi.bounding_box.y_max ); */
    /* for( j = 0; j < aoi.points.count; j++ ) */
    /* { */
    /*     printf( "[%f, %f], ", aoi.points.items[j][0], aoi.points.items[j][1] ); */
    /* } */
    /* printf( "\n" ); */

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
        compute( count, &h, fp_fixations, fp_saccades, fp_aoi );

        // perform analysis by computing 2d data from screen coordinates
        count = gac_sample_window_update( &h_screen,
                atof( vals[5] ), atof( vals[6] ), atof( vals[7] ),
                atof( vals[2] ), atof( vals[3] ), atof( vals[4] ),
                atof( vals[8] ), atoi( vals[9] ), vals[10] );
        compute( count, &h_screen, fp_fixations_screen,
                fp_saccades_screen, fp_aoi_screen );

next_loop:
        for( i = 0; i < 100; i++ )
        {
            free( vals[i] );
        }
    }

    res = gac_finalise( &h, &analysis );
    if( res )
    {
        write_aoi( &analysis, fp_aoi );
    }

    res = gac_finalise( &h_screen, &analysis );
    if( res )
    {
        write_aoi( &analysis, fp_aoi_screen );
    }

    // cleanup
    gac_destroy( &h );
    gac_destroy( &h_screen );
    fclose( fp );
    fclose( fp_fixations );
    fclose( fp_saccades );
    fclose( fp_aoi );
    fclose( fp_fixations_screen );
    fclose( fp_saccades_screen );
    fclose( fp_aoi_screen );

    return 0;
}
