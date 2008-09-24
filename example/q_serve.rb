require 'quail'
require 'quail/drb'

class TimeServer
  def current_time
    Time.now
  end
end

DRb.start_service('quail://localhost:5000/TS', TimeServer.new)
DRb.thread.join
