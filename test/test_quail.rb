require File.expand_path(File.join(File.dirname(__FILE__), "helper"))

class TestQuail < Quail::TestCase
  def test_constant_global
    assert_equal(1, Quail::GLOBAL)
  end

  def test_constant_local
    assert_equal(0, Quail::LOCAL)
  end
end
