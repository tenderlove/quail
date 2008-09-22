require File.expand_path(File.join(File.dirname(__FILE__), "..", "helper"))

module Quail
  class TestHandle < Quail::TestCase
    def test_initialize
      assert_nothing_raised {
        Quail::Handle.new('localhost.local')
      }
    end
  end
end
