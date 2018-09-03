CFLAGS = -std=c11 -ggdb3 -O2
LIBFLAGS = -fPIC
CXXFLAGS = -std=c++11 -ggdb3 -O2
LDFLAGS = -L./ -lqht -lpthread

ifeq (${STATIC}, 1)
	LDFLAGS+=-static
endif

ifeq (${RISCV}, 1)
	CC=riscv64-unknown-linux-gnu-gcc
	CXX=riscv64-unknown-linux-gnu-g++
	CFLAGS+=-march=rv64imafd
	CXXFLAGS+=-march=rv64imafd
endif

all: libqht bench
.phony: all

%.o: %.c qht.h
	$(CC) -c $< $(CFLAGS) $(LIBFLAGS)

%.o: %.cc qht.h
	$(CXX) -c $< $(CXXFLAGS)

libqht: qht.o
	$(CC) -shared -o $@.so $<
	ar rcs $@.a $<

bench: bench.o libqht
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f libqht.* *.o
