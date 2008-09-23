require 'quail'

####
# Chat server using Quail.
# 
# This file creates a "Quail" chat room.  A GLOBAL queue is created to receive
# messages from clients.  Clients bind their LOCAL exchange to this GLOBAL
# exchange in order to send messages.
#
# A GLOBAL exchange is created to broadcast messages.  Clients bind a LOCAL
# queue where they listen for messages.
#
# The error_handler is set to return true so that any client disconnects will
# not cause the server to exit.
#
# See irc_prompt.rb and irc_display.rb
#
Quail.error_handler = lambda { |name| true }

Quail::Handle.start('localhost') do |handle|
  exchange  = Quail::Exchange.new(handle, 'E_Quail', Quail::GLOBAL)
  queue     = Quail::Queue.new(handle, 'Q_Quail', Quail::GLOBAL)

  while f = queue.receive
    exchange.send "#{Time.now}: #{f}"
  end
end
