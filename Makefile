OBJS =  http_server.o utils/helper.o
CFLAGS = -Wall -g

all: ${OBJS}
	gcc ${CFLAGS} ${OBJS} -o start_server

clean:
	rm -f *.o utils/*.o start_server