CFLAGS = -std=c11 -ggdb3 -O2
LIBFLAGS = -fPIC
CXXFLAGS = -std=c++11 -ggdb3 -O2
LDFLAGS = -lqht -lpthread -L./

all: libqht bench
.phony: all

%.o: %.c qht.h
	$(CC) -c $< $(CFLAGS) $(LIBFLAGS)

%.o: %.cc qht.h
	$(CXX) -c $< $(CXXFLAGS)

libqht: qht.o
	$(CC) -shared -o $@.so $<

bench: bench.o libqht
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f libqht.so *.o
