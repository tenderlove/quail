require 'quail'

####
# Chat prompt using Quail.
#
# This file is the "prompt" for a chat client.  It creates a LOCAL exchange
# and binds it to the server's GLOBAL queue.  Messages are then written to
# the LOCAL exchange, and the server picks them up.
#
# See irc_server.rb
Quail::Handle.start('localhost') do |handle|
  exchange = Quail::Exchange.new(handle, 'E_Local')

  handle.bind('E_Local' => 'Q_Quail')

  10.times {
    exchange.send "Hello world"
    sleep 1
  }
end
