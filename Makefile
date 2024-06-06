# Variable to hold the testcase number
TESTCASE ?=

all: main runall

main: main.cpp
	g++ main.cpp -o main

run: main
ifeq ($(TESTCASE),)
	@echo "Please provide a testcase number, e.g., make run TESTCASE=testcasex or use runall to run all tests."
else
	./main < ./input/testcase$(TESTCASE).in > ./output/testcase$(TESTCASE).out
endif

runall: main
	for infile in ./input/*.in; do \
		base=$$(basename $$infile .in); \
		./main < $$infile > ./output/$$base.out; \
	done

clean:
	rm -f main
