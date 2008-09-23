require 'quail'

Quail::Handle.start('localhost') do |handle|
  exchange = Quail::Exchange.new(handle, 'EL', Quail::LOCAL)

  handle.bind("EL" => "QG")

  t = Time.now.to_f
  1_000.times { exchange.send('Hello world') }
  puts Time.now.to_f - t
end

