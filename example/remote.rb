require 'quail'

Quail::Handle.start('localhost') do |handle|
  exchange = Quail::Exchange.new(handle, 'EG', Quail::GLOBAL)
  queue = Quail::Queue.new(handle, 'QG', Quail::GLOBAL)

  while true
    handle.receive
  end
end
