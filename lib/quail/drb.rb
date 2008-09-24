require 'drb/drb'
require 'uri'
require 'stringio'
require 'quail/drb/client'

DRb::DRbServer.verbose = true
Thread.abort_on_exception = true

module Quail
  module DRb
    class << self
      def open uri, config
        raise(DRbBadScheme, uri) unless uri =~ /^quail/
        return Client.new(uri, config)
      end

      def uri_option uri, config
        return uri, nil
      end

      def open_server uri, config
        raise(DRbBadScheme, uri) unless uri =~ /^quail/
        return Server.new(uri, config)
      end
    end

    class Server
      attr_accessor :uri
      def initialize uri, config
        Quail.error_handler = lambda { |name|
          true
        }
        @uri = URI.parse(uri)
        @handle = Quail::Handle.new(@uri.host)
        @msg = ::DRb::DRbMessage.new(config)
        @config = config
        @handle.start unless @handle.started?
        @exchange = Quail::Exchange.new(  @handle,
                                          "#{@uri.path}_E",
                                          Quail::GLOBAL,
                                          "127.0.0.1:#{@uri.port}"
                                       )
        @queue = Quail::Queue.new(  @handle,
                                    "#{@uri.path}_Q",
                                    Quail::GLOBAL,
                                    "127.0.0.1:#{@uri.port + 1}"
                                 )
        Thread.main[:q] = @queue
        Thread.main[:e] = @exchange
      end

      def close
        #@handle.finish
      end

      def accept
        ServerSide.new(@config)
      end
    end

    class ServerSide
      def initialize config
        @msg = ::DRb::DRbMessage.new(config)
      end

      def recv_request
        Thread.critical = true
        result = @msg.recv_request(StringIO.new(Thread.main[:q].receive))
        result
      end

      def close
        Thread.critical = false
      end

      def send_reply succ, result
        stream = StringIO.new
        @msg.send_reply(stream, succ, result)
        stream.rewind
        Thread.main[:e].send stream.read
      end
    end
  end

  ::DRb::DRbProtocol.add_protocol(Quail::DRb)
end
