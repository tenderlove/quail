module Quail
  class Handle
    class << self
      def start host, &block
        new(host).start(host, &block)
      end
    end

    attr_accessor :host, :handle
    def initialize host
      @host = host
      @started = false
      @handle = nil
    end

    def started?
      @started
    end

    def start host = @host, &block
      create_handle(host)
      @started = true
      block.call(self)
      finish
    end

    def finish
      destroy_handle
      @started = false
    end

    def bind exchange, queue
      raise "not started" unless @started
      native_bind(exchange.name, queue.name)
    end
  end
end
