CC = cc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lpthread

OBJS_COMMON = crypto.o net.o

all: server client

server: server.o $(OBJS_COMMON)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

client: client.o $(OBJS_COMMON)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o server client

.PHONY: all clean


