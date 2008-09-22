INCLUDES        = -I$(top_builddir) -I$(top_builddir)/zmq \
                -I$(top_srcdir) -I$(top_srcdir)/zmq

noinst_PROGRAMS = gtw me stat

gtw_SOURCES = gtw.cpp ../../zmq/dispatcher.hpp ../../zmq/api_thread.hpp \
../../zmq/poll_thread.hpp messages.hpp frequency_meter.hpp common.hpp
gtw_LDADD = $(top_builddir)/zmq/libzmq.la
gtw_CXXFLAGS = -Wall -pedantic -Werror

me_SOURCES = me.cpp ../../zmq/dispatcher.hpp ../../zmq/api_thread.hpp \
../../zmq/poll_thread.hpp messages.hpp frequency_meter.hpp \
matching_engine.hpp common.hpp
me_LDADD = $(top_builddir)/zmq/libzmq.la
me_CXXFLAGS = -Wall -pedantic -Werror

stat_SOURCES = stat.cpp ../../zmq/dispatcher.hpp ../../zmq/api_thread.hpp \
../../zmq/poll_thread.hpp messages.hpp common.hpp 
stat_LDADD = $(top_builddir)/zmq/libzmq.la
stat_CXXFLAGS = -Wall -pedantic -Werror
