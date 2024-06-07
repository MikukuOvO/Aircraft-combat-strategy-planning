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

check: main
ifeq ($(TESTCASE),)
	@echo "Please provide a testcase number, e.g., make check TESTCASE=testcasex."
else
	./checker ./input/testcase$(TESTCASE).in ./output/testcase$(TESTCASE).out > ./report/report$(TESTCASE).log
endif

clean:
	rm -f main
	rm -f ./output/*.out
	rm -f ./report/*.log
