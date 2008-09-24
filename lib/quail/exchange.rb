module Quail
  class Exchange
    attr_accessor :eid, :name, :handle

    def initialize  handle,
                    exchange_name = 'E',
                    scope = Quail::LOCAL,
                    address = '127.0.0.1:5555'

      raise unless handle.started?
      @handle = handle.handle
      @name = exchange_name
      @eid = create_exchange(@handle, exchange_name, scope, address)
    end
  end
end
