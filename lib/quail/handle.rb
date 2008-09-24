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

    def start host = @host
      create_handle(host) unless started?
      @started = true

      if block_given?
        yield self
        finish
      end
    end

    def finish
      destroy_handle if started?
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
