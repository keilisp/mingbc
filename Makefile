##
# Mingbc
#
# @file
# @version 0.1
OBJS	= gc.o
SOURCE	= gc.c
OUT		= gc
CC		= gcc
FLAGS	= -std=c99 -g -c -Wall -Wextra -pedantic 

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT)

gc.o: gc.c
	$(CC) $(FLAGS) gc.c

clean:
	rm -f $(OBJS) $(OUT)

# end
