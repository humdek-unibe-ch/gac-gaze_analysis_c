TEST_DIRS = $(wildcard ./test_*/.)
TESTS = $(wildcard ./test_*/test)

all: $(TEST_DIRS)

.PHONY: $(TEST_DIRS)

$(TEST_DIRS):
	$(MAKE) run -C $@
