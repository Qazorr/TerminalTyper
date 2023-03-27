CO = g++
FLAGS = -Wall -pedantic
TO_CLEAN = *.x

src = main.cpp
obj = $(src:.cpp=.o)
exe = $(src:.cpp=.x)

.PHONY: clean

%.o: %.cpp
	@$(CO) $^ $(FLAGS) -c

%.x: %.cpp
	@$(CO) $< $(FLAGS) -o $@

compile: $(exe)
	@echo Files: {$(exe)} created.

run: $(exe)
	./$(exe)

clean:
	@rm $(TO_CLEAN)
	@echo Files: {$(exe)} removed.