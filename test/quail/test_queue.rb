require File.expand_path(File.join(File.dirname(__FILE__), "..", "helper"))

module Quail
  class TestQueue < Quail::TestCase
    def test_initialize
      assert_nothing_raised {
        Quail::Handle.start('localhost') do |handle|
          queue = Quail::Queue.new(handle)
        end
      }
    end
  end
end
