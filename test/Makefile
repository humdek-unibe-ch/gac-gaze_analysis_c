# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

TEST_DIRS = $(wildcard ./test_*/.)
TESTS = $(wildcard ./test_*/test)

all: $(TEST_DIRS)

.PHONY: $(TEST_DIRS)

$(TEST_DIRS):
	$(MAKE) run -C $@
