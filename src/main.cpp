
#include <iostream>
#include <VkContext.hpp>
#include <memory>
using namespace std;
int main(){

  auto instance = std::make_shared<VkInstanceObject>();
  auto physicalDevice = std::make_shared<VkPhysicalDeviceObject>(instance);
  const auto & f = physicalDevice->GetPhysicalDeviceFeatures();
  return 0;
}
