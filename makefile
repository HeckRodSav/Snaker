GPP = g++
ADDITIONAL = 
CFLAGS = $(ADDITIONAL) -march=native -std=c++14 -ansi -pedantic -Wall -Werror -lm

SOURCE = snaker.cpp
RUNNER = snaker.run

.PHONY: compile
compile: $(SOURCE)
	$(GPP) $< -o $(RUNNER) $(CFLAGS)

.PHONY: run
run: $(RUNNER)
	./$(RUNNER)