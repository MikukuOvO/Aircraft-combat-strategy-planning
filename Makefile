# Variable to hold the testcase number
TESTCASE ?=

all: main

main: main.cpp
	g++ main.cpp -o main

run: main
ifeq ($(TESTCASE),)
	@echo "Please provide a testcase number, e.g., make run TESTCASE=testcasex or use runall to run all tests."
else
	./main $(TESTCASE) < ./input/testcase$(TESTCASE).in > ./output/testcase$(TESTCASE).out
endif

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
