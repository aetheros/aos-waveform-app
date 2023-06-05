
-include config.mk

CXXFLAGS += -MMD -MP -Ilib
LIBS += -lsdk_aos -lcoap -ldtls -lcommon -lappfw

EXE = aos_waveform_app

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

all: $(EXE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS) $(LIBS)

-include $(DEPS)

clean:
	rm -f $(EXE) $(OBJS) $(DEPS) *.aos

.PHONY: all clean

