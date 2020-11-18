include ../../../config.mak
include config.mak

CFLAGS += $(shell pkg-config mlt-framework --cflags)
CXXFLAGS += $(shell pkg-config mlt-framework --cflags)

CFLAGS += -I../..

CXXFLAGS := $(QTCXXFLAGS) $(CXXFLAGS) $(CFLAGS) -fPIC

ifneq (, $(shell $(CXX) --version | grep -is g++))
    CXXFLAGS += --std=c++11
endif

LDFLAGS += $(QTLIBS) -lmlt -lm

TARGET = ../libmltrl$(LIBSUF)

OBJS = factory.o

CPPOBJS = filter_typewriter.o typewriter.o

SRCS := $(OBJS:.o=.c) $(CPPOBJS:.o=.cpp)

all: 	$(TARGET)

$(TARGET): $(OBJS) $(CPPOBJS)
		$(CXX) $(SHFLAGS) -o $@ $(OBJS) $(CPPOBJS) $(LDFLAGS)

depend:	$(SRCS)
		$(CXX) -MM $(CXXFLAGS) $^ 1>.depend

distclean:	clean
		rm -f .depend

clean:	
		rm -f $(OBJS) $(CPPOBJS) $(TARGET)

install: all
		install -m 755 $(TARGET) "$(DESTDIR)$(moduledir)"
		install -d "$(DESTDIR)$(mltdatadir)/rl"
		install -m 644 *.yml "$(DESTDIR)$(mltdatadir)/rl"

ifneq ($(wildcard .depend),)
include .depend
endif
