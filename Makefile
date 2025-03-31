CC=gcc
CFLAGS=-O3 -Wall -Wextra

GRADER=rtgrader
EXE=jplc
RT=jplc-linux
CACHE=cacher
TEST=hw2

all: $(CACHE) $(GRADER)

$(GRADER): rtgrader.c vector.c vector.h cvec.c cvec.h
	$(CC) -o $@ $^ $(CFLAGS)

$(CACHE): cacher.c vector.c vector.h cvec.c cvec.h
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	@rm -f *.txt
	@rm -f *.out
	@rm -f _temp*
	@rm -f cacher
	@rm -f rtgrader
	@rm -f *.out
	@find . -type f -name "*.Identifier" -delete
	@find . -type f -name "callgrind.*" -delete

clean-expected:
	@rm -r expected

$(TEST_LIST):

run: $(GRADER)
	./$(GRADER) $(EXE) $(TEST)

cache: $(CACHE) 
	./$(CACHE) $(RT) $(TEST)

run-all: $(GRADER)
	./rtgrader $(EXE) hw2
	./rtgrader $(EXE) hw3
	./rtgrader $(EXE) hw4
	./rtgrader $(EXE) hw5
	./rtgrader $(EXE) hw6
	./rtgrader $(EXE) hw8
	./rtgrader $(EXE) hw10
	./rtgrader $(EXE) hw11

cache-all: $(CACHE)
	./cacher $(RT) hw2
	./cacher $(RT) hw3
	./cacher $(RT) hw4
	./cacher $(RT) hw5
	./cacher $(RT) hw6
	./cacher $(RT) hw8
	./cacher $(RT) hw10
	./cacher $(RT) hw11

help:
	@echo "\tmake: builds the cacher and autograder executables\n"
	@echo "\tmake run [EXE] [TEST]: runs the autograder, using executable [EXE] (default 'jplc'), on test-folder [TEST] (default 'hw2')"
	@echo "\tmake run-all [EXE]: runs the autograder, using executable [EXE] (default 'jplc'), on all test-folders (excluding 7, 9, 12, 13, 14)\n"
	@echo "\tmake cache [RT] [TEST]: runs the cacher, using the runtime compiler [RT] (default 'jplc-linux'), on test-folder [TEST] (default 'hw2')"
	@echo "\tmake cache-all [RT]: runs the cacher, using the runtime compiler [RT] (default 'jplc-linux'), on all test-folders (excluding 7, 9, 12, 13, 14)\n"
	@echo "\tmake clean: cleans up all non-src files in the folder, including the executables. Does not remove the 'expected' folder"
	@echo "\tmake clean-expected: deletes the expected folder; unadvised due to slow execution time of cacher"
