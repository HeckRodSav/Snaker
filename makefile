GPP = g++
ADDITIONAL = 
CFLAGS = $(ADDITIONAL) -ansi -pedantic -Wall -Werror -lm

SOURCE = snaker.cpp
RUNNER = snaker.run

.PHONY: compile
compile: $(SOURCE)
	$(GPP) $< -o $(RUNNER) $(CFLAGS)

.PHONY: run
run: $(RUNNER)
	./$(RUNNER)