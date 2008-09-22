# -*- ruby -*-

require 'rubygems'
require 'hoe'
require './lib/quail/version'

kind = Config::CONFIG['DLEXT']

EXT = "ext/quail/native.#{kind}"
HOE = Hoe.new('quail', Quail::VERSION) do |p|
  p.developer('Aaron Patterson', 'aaronp@rubyforge.org')
  p.clean_globs = [
    'ext/quail/Makefile',
    'ext/quail/*.{o,so,bundle,a,dylib,log}',
  ]
end

namespace :build do
  file 'ext/quail/Makefile' do
    Dir.chdir('ext/quail') do
      ruby 'extconf.rb'
    end
  end

  file EXT => ['vendor/lib/libzmq.a', 'ext/quail/Makefile'] do
    Dir.chdir('ext/quail') do
      sh 'make'
    end
  end

  file "vendor/lib/libzmq.a" do
    prefix = File.join(File.dirname(__FILE__), 'vendor')
    Dir.chdir('vendor/zmq-0.3') do
      sh "./configure --prefix=#{prefix}"
      sh "make"
      sh "make install"
    end
  end

  task :all => [EXT]
end

task :build => ["build:all"]

# vim: syntax=Ruby
