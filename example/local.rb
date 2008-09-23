require 'quail'

msg = "hello world"
Quail::Handle.start('localhost') do |handle|
  exchange = Quail::Exchange.new(handle, 'EL', Quail::LOCAL)

  handle.bind("EL", "QG")

  t = Time.now.to_f
  1_000_000.times {
    exchange.send(msg)
  }
  puts Time.now.to_f - t
  sleep 2
end

