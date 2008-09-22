module Quail
  class Queue
    attr_accessor :name

    def initialize  handle,
                    name    = 'Q',
                    scope   = Quail::GLOBAL,
                    address = '127.0.0.1:5001'

      @handle = handle
      @name = name
      create_queue(handle, name, scope, address)
    end
  end
end
