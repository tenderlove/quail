require 'quail/native'
require 'quail/exchange'
require 'quail/queue'
require 'quail/handle'

module Quail
  class << self
    attr_accessor :error_handler
  end
end

Quail.error_handler = lambda { |name|
  false
}
