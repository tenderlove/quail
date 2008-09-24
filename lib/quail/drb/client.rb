module Quail
  module DRb
    class Client
      def initialize uri, config
        @uri = URI.parse(uri)
        @handle = Quail::Handle.new(@uri.host)
        @msg = ::DRb::DRbMessage.new(config)
      end

      def close
        @handle.finish
      end

      def alive?
        @handle.started?
      end

      def send_request ref, msg_id, *arg, &b
        connect

        stream = StringIO.new
        @msg.send_request(stream, ref, msg_id, *arg, &b)
        stream.rewind
        @exchange.send stream.read
      end

      def recv_reply
        @msg.recv_reply(StringIO.new(@queue.receive))
      end

      private
      def connect
        unless @handle.started?
          @handle.start
          @exchange = Quail::Exchange.new(@handle, exchange_name)
          @handle.bind(exchange_name => "#{@uri.path}_Q")
          @queue = Quail::Queue.new(@handle, queue_name)
          @handle.bind("#{@uri.path}_E" => queue_name)
        end
      end

      def exchange_name
        "#{@uri.path}_E_#{self.object_id}"
      end

      def queue_name
        "#{@uri.path}_Q_#{self.object_id}"
      end
    end
  end
end
