CFLAGS += $(shell pkg-config mlt-framework --cflags)
CXXFLAGS += $(shell pkg-config mlt-framework --cflags)

CFLAGS += -I../..

CXXFLAGS := $(CXXFLAGS) $(CFLAGS) -fPIC

ifneq (, $(shell $(CXX) --version | grep -is g++))
    CXXFLAGS += --std=c++11
endif

LDFLAGS += -L../../framework -lmlt -lm

include ../../../config.mak

TARGET = ../libmltrl$(LIBSUF)

OBJS = factory.o \
	   filter_typewriter.o

CPPOBJS = typewriter.o \
	   parsing_functions.o \
	   c_functions.o

SRCS := $(OBJS:.o=.c) $(CPPOBJS:.o=.cpp)

all: 	$(TARGET)

$(TARGET): $(OBJS) $(CPPOBJS)
		$(CXX) $(SHFLAGS) -o $@ $(OBJS) $(CPPOBJS) $(LDFLAGS)

depend:	$(SRCS)
		$(CXX) -MM $(CXXFLAGS) $^ 1>.depend

distclean:	clean
		rm -f .depend

clean:	
		rm -f $(OBJS) $(TARGET)

install: all
		install -m 755 $(TARGET) "$(DESTDIR)$(moduledir)"
		install -d "$(DESTDIR)$(mltdatadir)/rl"
		install -m 644 *.yml "$(DESTDIR)$(mltdatadir)/rl"

ifneq ($(wildcard .depend),)
include .depend
endif
