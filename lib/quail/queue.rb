module Quail
  class Queue
    attr_accessor :name

    def initialize  handle,
                    name    = 'Q',
                    scope   = Quail::LOCAL,
                    address = '127.0.0.1:5556'

      raise unless handle.started?
      @handle = handle.handle
      @name = name
      create_queue(@handle, name, scope, address)
    end
  end
end
