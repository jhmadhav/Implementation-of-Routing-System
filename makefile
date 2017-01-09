

CC = gcc

CFLAGS = -Wall -g

INCLUDES = -I/.



SRCS = hemamadh.c

OBJS = $(SRCS:.c=.o)

MAIN = server




.PHONY: depend clean


all:    $(MAIN)
	

$(MAIN): $(OBJS) 
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS)


.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)


