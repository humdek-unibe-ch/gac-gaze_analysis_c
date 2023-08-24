# Example Code

The example code shows how to parse a csv file of gaze samples.

The input file is processed twice:
Once by reading the 2d data points from the input file and propagating them to the fixation and saccade structures and once by computing the 2d data points through (hard coded) screen coordinates.

The example produces the following ouput files:
- `fixations.csv`: holds all fixations where the 2d data points were propagated from the input file.
- `saccades.csv`: holds all saccades where the 2d data points were propagated from the input file.
- `fixations_screen.csv`: holds all fixations where the 2d data point were computed by the library with the help of screen coordinates.
- `saccades_screen.csv`: holds all saccades where the 2d data point were computed by the library with the help of screen coordinates.

## Build

To build the example use

```sh
make
```

To run the example run

```sh
make run
```

To check for memory leaks run

```sh
make valgrind
```
