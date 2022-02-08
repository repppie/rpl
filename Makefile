PROG = rpl

SRCS = rpl.c y.tab.c
HEADERS =
OBJS = $(SRCS:.c=.o)

CC = gcc
YACC = yacc
CFLAGS = -Wall -g
#LDFLAGS += -lncurses

all: $(PROG)

clean:
	rm -f $(OBJS) $(PROG)

$(PROG): $(OBJS) $(HEADERS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

y.tab.o: y.tab.c
y.tab.c: y.y
	yacc y.y
