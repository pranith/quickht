CFLAGS = -std=c11 -fPIC -O0 -g
LDFLAGS = -lqht -L./

all: libqht bench
.phony: all

%.o: %.c
	$(CC) -c $< $(CFLAGS)

libqht: qht.o
	$(CC) -shared -o $@.so $<

bench: bench.o libqht
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f libqht.so *.o
