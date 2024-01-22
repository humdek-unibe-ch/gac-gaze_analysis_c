# Changelog

-------------------
## `v0.3.0` (latest)

### Changes

* Change the formula to compute the dispersion (#1).


-------------------
## `v0.2.3`

### Bug Fixes

* Fix dispersion calculation.


-------------------
## `v0.2.2`

### New Features

* Add helper functions to compute timestamps and onsets.

### Improvements

* Add non-rectangular AOI to the example.
* Improve reported AOI timestamp information.

### Bug Fixes

* Fix trial ID and label onset calculations.


-------------------
## `v0.2.1`

### Improvements

* Add aoic structure to gaze handler.

### Bug Fixes

* Fix memory leak.


-------------------
## `v0.2.0`

### New Features

* Add support for screen resolution (internally 2d coordinates are still stored
  as normalized values).
* Allow to define area of interests (AOI) and perform a basic analysis based on
  fixations and saccades.

### Improvements

* Add MPL license.
* Add this changelog.
* Use a minimalistic include approach with cglm instead of including everything.
* Split code into individual file pairs (.c and .h) to separate concerns.

### Bug Fixes
* Fix doxygen configuration.


-------------------
## `v0.1.1`

Initial release.

