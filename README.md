# GAC - Gaze Analysis C Library

This is a pure C library to perform basic gaze analysis.

Features:
- Sample filtering with moving average
- Sample gap fill-in through linear interpolation (lerp)
- Fixation detection with I-DT algorithm
- Saccade detection with I-VT algorithm

## Quick Start

Initialise the gaze analysis handler:

```c
gac_t h;
gac_init( &h, NULL );
```

To parse gaze data for fixations and saccades, for each new sample do the following:

```c
gac_sample_window_update( &h, sample.origin.x, sample.origin.y, sample.oridin.z,
        sample.point.x, sample.point.y, sample.point.z, sample.timestamp );

// check for fixation
res = gac_sample_window_fixation_filter( &h, &fixation );
if( res == true )
{
    // new fixation deteced
}

// check for saccade
res = gac_sample_window_saccade_filter( &h, &saccade );
if( res == true )
{
    // new saccade detected
}
```

At the end, destroy the gaze analysis handler:
```c
gac_destroy( &h )
```
