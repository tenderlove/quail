INCLUDES        = -I$(top_builddir) -I$(top_builddir)/zmq \
                -I$(top_srcdir) -I$(top_srcdir)/zmq


noinst_PROGRAMS = receiver sender

receiver_SOURCES = receiver.cpp
receiver_LDADD = $(top_builddir)/zmq/libzmq.la
receiver_LDFLAGS = @SDL_LIBS@
receiver_CXXFLAGS = -Wall @SDL_CFLAGS@ -Werror -Wextra

sender_SOURCES = sender.cpp ./ucil.h
sender_LDADD = $(top_builddir)/zmq/libzmq.la
sender_LDFLAGS = @UNICAP_LIBS@ @UCIL_LIBS@
sender_CXXFLAGS = -Wall @UNICAP_CFLAGS@ @UCIL_CFLAGS@ -Wextra -Werror
