require File.expand_path(File.join(File.dirname(__FILE__), "..", "helper"))

module Quail
  class TestExchange < Quail::TestCase
    def test_initialize
      assert_nothing_raised {
        Quail::Handle.start('localhost') { |handle|
          exchange = Quail::Exchange.new(handle)
        }
      }
    end
  end
end
