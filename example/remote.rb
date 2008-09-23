require 'quail'

Quail::Handle.start('localhost') do |handle|
  queue = Quail::Queue.new(handle, 'QG', Quail::GLOBAL)

  while true
    puts handle.receive
  end
end
