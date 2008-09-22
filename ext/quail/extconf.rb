ENV["ARCHFLAGS"] = "-arch #{`uname -p` =~ /powerpc/ ? 'ppc' : 'i386'}"

require 'mkmf'

$CFLAGS << " -dynamiclib -bundle -flat_namespace -undefined suppress -pthread -fPIC "
vendor_dir = File.expand_path(
  File.join(File.dirname(__FILE__), '../', '../', 'vendor')
)

$LOCAL_LIBS << "#{File.join(vendor_dir, 'lib', 'libzmq.a')}"

$libs = append_library($libs, "stdc++")
find_header('zmq/message.hpp', "#{File.join(vendor_dir, 'include')}")

create_makefile('quail/native')
