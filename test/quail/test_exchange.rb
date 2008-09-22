require File.expand_path(File.join(File.dirname(__FILE__), "..", "helper"))

module Quail
  class TestExchange < Quail::TestCase
    def test_initialize
      handle = Quail::Handle.new('localhost')
      assert_nothing_raised {
        exchange = Quail::Exchange.new(handle)
      }
    end
  end
end
