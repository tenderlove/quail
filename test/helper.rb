require 'test/unit'

%w(../lib ../ext).each do |path|
  $LOAD_PATH.unshift(File.expand_path(File.join(File.dirname(__FILE__), path)))
end

Thread.new { `vendor/bin/zmq_server` }

require 'quail'

module Quail
  class TestCase < Test::Unit::TestCase
    undef :default_test
  end
end
