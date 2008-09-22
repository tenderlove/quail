require File.expand_path(File.join(File.dirname(__FILE__), "..", "helper"))

module Quail
  class TestHandle < Quail::TestCase
    def test_initialize
      assert_nothing_raised {
        Quail::Handle.new('localhost.local')
      }
    end

    def test_bind
      Quail::Handle.start('localhost') do |handle|
        exchange = Quail::Exchange.new(handle)
        queue = Quail::Queue.new(handle)
        handle.bind(exchange, queue)
      end
    end
  end
end
