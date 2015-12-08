###############################################################################

## COMPILER ##
CXX       = g++

## FLAGS ##
CXXFLAGS  = -Wall -Wextra -Werror -pedantic  -std=c++11  -I $(INCLDIR)
CXXFLAGS += -pedantic -g
#CXXFLAGS += -Wundef -Wshadow -Wpointer-arith -Wcast-qual
#CXXFLAGS += -Wcast-align
#CXXFLAGS += -Wmissing-declarations
#CXXFLAGS += -Wunreachable-code
CXXFLAGS += -fdiagnostics-color=always

## INCLUDES DIRECTORY ##
INCLDIR   = src/includes/

## MAIN ##
SRCS     = $(addsuffix .cc, $(addprefix src/, main game))

## OBJ CREATION ##
OBJS      = $(SRCS:.cc=.o)

## EXEC NAME ##
EXEC      = game

###############################################################################

# Multi threaded make of the final binary #
multi:
	$(MAKE) -j all

# Produce the final binary   #
all: $(OBJS)
	$(CXX) $(OBJS) $(LDLIBS) -o $(EXEC)

# Produce test binary, and launch #
check: multi
	./$(EXEC)

# Clean repository           #
clean:
	$(RM) $(OBJS) $(EXEC)

.PHONY: multi all check clean

###############################################################################
