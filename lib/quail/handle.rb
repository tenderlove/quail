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

    ####
    # Bind these +connections+
    #   handle.bind(
    #     'EXCHANGE_LOCAL' => 'QUEUE_GLOBAL',
    #     'EXCHANGE_LOCAL' => 'QUEUE_GLOBAL',
    #   )
    def bind connections
      raise "not started" unless @started
      connections.each do |k,v|
        native_bind(k, v)
      end
    end
  end
end
