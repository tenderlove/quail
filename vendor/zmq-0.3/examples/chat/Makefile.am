INCLUDES        = -I$(top_builddir) -I$(top_builddir)/zmq \
                -I$(top_srcdir) -I$(top_srcdir)/zmq

noinst_PROGRAMS = chatroom display prompt

chatroom_SOURCES = chatroom.cpp 
chatroom_LDADD = $(top_builddir)/zmq/libzmq.la
chatroom_CXXFLAGS = -Wall -pedantic -Werror

display_SOURCES = display.cpp
display_LDADD = $(top_builddir)/zmq/libzmq.la
display_CXXFLAGS = -Wall -pedantic -Werror

prompt_SOURCES = prompt.cpp 
prompt_LDADD = $(top_builddir)/zmq/libzmq.la
prompt_CXXFLAGS = -Wall -pedantic -Werror
