require 'quail'

####
# Chat display using Quail.
#
# This file displays messages sent to a chat server using Quail.  It creates
# a LOCAL queue, and binds it to the server's GLOBAL exchange.  The LOCAL queue
# gets messages from the GLOBAL exchange and displays them.
#
# See irc_server.rb and irb_prompt.rb
#
Quail::Handle.start('localhost') do |handle|
  queue = Quail::Queue.new(handle, 'Q_Local', Quail::LOCAL)

  handle.bind('E_Quail' => 'Q_Local')

  while message = queue.receive
    puts message
  end
end
