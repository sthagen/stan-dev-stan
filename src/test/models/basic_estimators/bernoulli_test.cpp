#include <gtest/gtest.h>
#include <test/models/model_test_fixture.hpp>

class Models_BasicEstimators_Bernoulli : 
  public Model_Test_Fixture<Models_BasicEstimators_Bernoulli> {
protected:
  virtual void SetUp() {
  }
public:
  static std::vector<std::string> get_model_path() {
    std::vector<std::string> model_path;
    model_path.push_back("models");
    model_path.push_back("basic_estimators");
    model_path.push_back("bernoulli");
    return model_path;
  }
  static bool has_data() {
    return true;
  }
};

TEST_F(Models_BasicEstimators_Bernoulli,RunModel) {
  run_model();
}
