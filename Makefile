CC = gcc
CFLAGS = -Wall -g
SRCS = main.c command.c tokenize.c util.c prompt.c builtin.c
OBJS = $(SRCS:.c=.o)
TEST_SRCS = test.c
TEST_OBJS = $(TEST_SRCS:.c=.o)
LIBS = -lreadline -lhistory
INCS = 
PROGRAM = nihonsh

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) $^ $(CFLAGS) $(LIBS) $(INCS) -o $@

%.o: %.c
	$(CC) -c $< $(CFLAGS) $(LIBS) $(INCS)

.PHONY: test %.o
test: util.o command.o tokenize.o prompt.o test.o
	$(CC) $^ $(CFLAGS) $(LIBS) $(INCS) -o $@ && ./$@

.PHONY: clean
clean:
	rm -f $(OBJS)
	rm -f $(TEST_OBJS)
	rm -f $(PROGRAM)