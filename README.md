# GAC - Gaze Analysis C Library

This is a pure C library to perform basic gaze analysis.

Features:
- Sample filtering with moving average
- Sample gap fill-in through linear interpolation (lerp)
- Fixation detection with I-DT algorithm
- Saccade detection with I-VT algorithm
- Area of interest (AOI) analysis

## Quick Start

Initialise the gaze analysis handler:

```c
gac_t h;
gac_init( &h, NULL );
```

To perform an AOI analysis, add some AOIs to the gaze analysis handler:

```c
gac_aoi_t aoi;
gac_aoi_init( &aoi, "my_circular_aoi" );
gac_aoi_add_point( &aoi, 0.5, 0.4 );
gac_aoi_add_point( &aoi, 0.5, 0.3 );
gac_aoi_add_point( &aoi, 0.6, 0.2 );
gac_aoi_add_point( &aoi, 0.7, 0.2 );
gac_aoi_add_point( &aoi, 0.8, 0.3 );
gac_aoi_add_point( &aoi, 0.8, 0.4 );
gac_aoi_add_point( &aoi, 0.7, 0.5 );
gac_aoi_add_point( &aoi, 0.6, 0.5 );
gac_add_aoi( &h, &aoi );

gac_aoi_init( &aoi, "my_rectangular_aoi" );
gac_aoi_add_rect( &aoi, 0.3, 0.45, 0.1, 0.1 );
gac_add_aoi( &h, &aoi );
```

To parse gaze data for fixations and saccades, for each new sample do the following:

```c
int i, count;
bool res;
gac_fixation_t fixation;
gac_saccade_t saccade;
gac_aoi_collection_analysis_result_t analysis;

// update the sample window with data.
count = gac_sample_window_update( &h, sample.origin.x, sample.origin.y,
        sample.oridin.z, sample.point.x, sample.point.y, sample.point.z,
        sample.timestamp );

// updating the sample window may add multiple new samples (due to gap
// filtering). Hence, we need to filter for each sample added.
for( i = 0; i < count; i++ )
{
    // check for saccade
    res = gac_sample_window_saccade_filter( &h, &saccade );
    if( res == true )
    {
        // perform AOI analysis on saccade data
        gac_aoi_collection_analyse_saccade( &h.aoic, &saccade );
        // saccade structures must be destroyed once they are no longer needed.
        gac_saccade_destroy( &saccade );
    }

    // check for fixation
    res = gac_sample_window_fixation_filter( &h, &fixation );
    if( res == true )
    {
        // perform AOI analysis on fixation data.
        res = gac_aoi_collection_analyse_fixation( &h->aoic, &fixation,
                &analysis );
        if( res == true )
        {
            // An AOI analysis entry is ready, do something with the analysis
            // data
        }
        // fixation structures must be destroyed once they are no longer needed.
        gac_fixation_destroy( &fixation );
    }

    // remove samples from the sample window which are no longer used
    gac_sample_window_cleanup( h );
}
```

After all samples were analysed, a finalization step is required to conclude
the AOI analysis:
```c
bool res = gac_finalise( &h, &analysis );
if( res )
{
    // An AOI analysis entry is ready, do something with the analysis data
}
```

Finally, destroy the gaze analysis handler:
```c
gac_destroy( &h );
```

## Basic Concept

The library provides several functions to work with gaze data.
The easiest approach is to use the functions `gac_sample_window_*` as these maintain their own sample window and noise and gap filters can be configured through the `gac_filter_paramter_t` structure.

Alternatively it is possible to manually maintain a sample window and work with each filter individually. This means filter structures have to be created and destroyed manually and filtering has to be applied manually to a custom sample window.
Refer to the API for more information.

### Detection Algorithm

Fixations are detected with the I-DT algorithm (Salvucci & Goldberg 2000).
Saccades are detected with the I-VT algorithm (Salvucci & Goldberg 2000).

Note that the resulting fixations and saccades will **not** fit together perfectly (e.g. a saccade follows a fixation and vice versa) because
1. both algorithms work with their own parameters which will most likely lead to gaps (data which is neither classified as part of a fixation nor saccade)
2. gaze data may be a recording of a smooth pursuit
3. gaps in the gaze data because of blinks or other data loss

For more details on the filter parameter options refer to the API documentation.

### Filters

Optionally the gaze data is processed by
1. a moving average filter which computes the average of all samples in the filters own sliding window. Sample annotations (e.g. the label, trial ID, and timestamps) are copied from the data sample in the middle of the sliding window.
2. a gap fill-in filter where data samples are filled into gaps using linear interpolation.

For more details on the filter parameter options refer to the API documentation.

### 3d vs 2d Data

All calculations are performed on 3d data.
If only 2d data is available this library cannot be used (yet).
The reason for this is that with 3d data it is possible to compute an accurate dispersion and velocity threshold based on the distance of the gaze origin to the gaze point.
For 2d data the dispersion and velocity threshold would need to be estimated based on the measured data which is not (yet) supported by the library.

However, it is possible to provide 2d data alongside 3d data for each data sample which will propagated to fixation and saccade result structures.
To add 2d data for each sample instead of the function `gac_sample_window_update()` use `gac_sample_window_update_screen()`.

If 2d data is not available it is possible to compute it from 3d data.
`gac_sample_window_update()` does this automatically if the screen location is defined.
To define the screen location use the function `gac_set_screen()`.

### Sample annotations

Each sample has two fields available for custom data annotation:
 - `trial_id`: expects an integer number and can be used to e.g. associate a data point to a trial.
 - `label`: expects a string and can be used to e.g. describe the currently displayed stimuli.

The annotations are propagated to the fixation and saccade result structures.

Further, each sample has two additional timestamp fields for onset information of the annotations:
 - `trial_onset`: the amount of milliseconds since the last change in the field `trial_id`.
 - `label_onset`: the amount of milliseconds since the last change in the field `label`.

### Area of Interest (AOI) Analysis

The area of interest (AOI) analysis is performed based on fixations.
Saccade information can also be used to extend the analysis but fixations are always required.
For each distinct trial ID block an analysis of each AOI is performed.

To decide whether a sample point is inside an AOI a ray casting method is used where a virtual ray is drawn from an arbitrary point outside the AOI to the sample point.
Then, every intersection with segments of the AOI contour is counted.
If an even number of intersection is detected, the point lies outside of the AOI, otherwise the point lies inside the AOI.
To improve performance, a coarse detection using a rectangular a bounding box is performed (if the sample point lies outside the bounding box it also lies outside the AOI).


## Building the library on Linux (Ubuntu)

In order to build the library the following packages are required:

```sh
sudo apt install build-essential
sudo apt install autoconf autogen libtool
```

To build the library use the commands

```sh
autoreconf --install
./configure
make
```

To build and run tests use

```sh
cd test
make
```

To build and run the example use

```sh
cd example
make
make run
```

## Building the library on Windows

Build the library on windows with [`msys2`](https://www.msys2.org/).
Once installed start `msys2.exe`.

Some dependencies need to be installed.
To do this type the following commands:

```sh
pacman -Syyu
pacman -Sy mingw-w64-x86_64-gcc
pacman -Sy autogen autoconf automake libtool
```

Finally, to build the library type

```sh
make
```

Build the example with the following commands:

```sh
cd example
make
```

To run the example make sure that the system knows the location of `msys2.dll` (either by adding the location to the PATH or by copying the file to the example folder).
Run the example by starting `example.exe`.
