require 'quail'
require 'quail/drb'

ts = DRbObject.new_with_uri('quail://localhost/TS')
start = Time.now.to_f
3000.times { ts.current_time }
puts Time.now.to_f - start
