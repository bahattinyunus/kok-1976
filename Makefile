# KÖK 1976 - Bağımsız Standalone Makefile
# (Harici Bağımlılık Yoktur - Sadece C++17 Derleyicisi)

CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2 -Icore/include -Iradio/modulation -Iradio/telemetry -Iradio/audio -Iradio/network -Iradio/sdr

SRCS = core/src/math.cpp \
       core/src/fec.cpp \
       core/src/crypto.cpp \
       radio/modulation/fsk.cpp \
       radio/modulation/fhss.cpp \
       radio/modulation/dsss.cpp \
       radio/telemetry/frame.cpp \
       radio/telemetry/protocol.cpp \
       radio/audio/cvsd.cpp \
       radio/audio/squelch.cpp \
       radio/network/mesh_router.cpp \
       radio/sdr/dsp_filter.cpp

OBJS = $(SRCS:.cpp=.o)

all: libkok.a kok_cpp_test

libkok.a: $(OBJS)
	ar rcs $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

kok_cpp_test: tests/test_cpp_core.cpp libkok.a
	$(CXX) $(CXXFLAGS) $< -L. -lkok -o $@

test: kok_cpp_test
	./kok_cpp_test
	python -s run_tests.py

clean:
	rm -f $(OBJS) libkok.a kok_cpp_test
