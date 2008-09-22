require File.expand_path(File.join(File.dirname(__FILE__), "..", "helper"))

module Quail
  class TestQueue < Quail::TestCase
    def test_initialize
      handle = Quail::Handle.new('localhost')
      assert_nothing_raised {
        queue = Quail::Queue.new(handle)
      }
    end
  end
end
