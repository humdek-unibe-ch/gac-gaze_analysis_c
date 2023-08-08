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

// remove samples from the sample window which are no longer used
gac_sample_window_cleanup( h );
```

At the end, destroy the gaze analysis handler:
```c
gac_destroy( &h );
```

## Building the library on Linux (Ubuntu)

In order to build the library the following packages are required:

```sh
sudo apt install build-essential
sudo apt install autoconf autogen libtool
```

To build the library use the command

```sh
make
```

To run tests use

```sh
make test
```

Build and run the example with the following commands:

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
