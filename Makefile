CFLAGS = -std=c11 -fPIC

all: libqht
.phony: all

qht.o: qht.c
	$(CC) -c $< $(CFLAGS)

libqht: qht.o
	$(CC) -shared -o $@.so $<

clean:
	rm -f libqht.so *.o
