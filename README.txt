= Quail

* http://github.com/tenderlove/quail/tree/master

== DESCRIPTION:

Really Fucking Fast Messaging in ruby.

== FEATURES/PROBLEMS:

Don't forget to start vender/bin/zmq_server !

== SYNOPSIS:

  Quail::Handle.start('localhost') do |handle|
    exchange  = Quail::Exchange.new(handle)
    queue     = Quail::Queue.new(handle)
            
    handle.bind(exchange.name => queue.name)
          
    exchange.send 'Hello world'
    puts queue.receive
  end

== INSTALL:

* sudo gem install quail

== LICENSE:

(The MIT License)

Copyright (c) 2008 Aaron Patterson

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
'Software'), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
