#pragma once
#include <new>
#include <vector>
#include <iostream>
#include <memory>

#ifdef __linux__
#define VK_USE_PLATFORM_XCB_KHR
#elif _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>

const std::vector<const char *> validationLayers={};
const std::vector<const char *> deviceExtensions={VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#if !defined (NDEBUG)
const bool enableValidationLayer=true;
#else
const bool enableValidationLayer = false;
#endif


#define VK_CHECK(expr)  \
do{                     \
  VkResult res;\
  if( (res = expr) != VK_SUCCESS){ \
    std::cout<<"VkResult: ("<<res<<"): "<<#expr<<": Vulkan Assertion failed\n"; \
    exit(-1);\
  }\
}while(false);\

struct VkInstanceObject{
  VkInstanceObject(){
    using namespace std;
    uint32_t extPropCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extPropCount, nullptr);
    vector<VkExtensionProperties> props(extPropCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extPropCount, props.data());
    cout<<"Instance Extensions:"<<endl;
    for(int i = 0;i<props.size();i++){
      cout<<props[i].extensionName<<endl;
    }
    if(glfwInit()==GLFW_FALSE){
      cout<<"glfw init failed";
      exit(-1);
    }

    uint32_t extCount = 0;
    auto ext = glfwGetRequiredInstanceExtensions(&extCount);
    vector<const char *> extNames(ext,ext+extCount);

    std::cout<<"GLFW Required Instance"<<endl;
    for(const auto & item:extNames){
      cout<<item<<endl;
    }

    if(enableValidationLayer){
      extNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    VkInstanceCreateInfo CI={};
    CI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    CI.pNext = nullptr;
    CI.enabledExtensionCount = extNames.size();
    CI.ppEnabledExtensionNames = extNames.data();

    CI.pApplicationInfo = nullptr;
    
    if(enableValidationLayer)
    {
      CI.enabledLayerCount = validationLayers.size();
      CI.ppEnabledLayerNames = validationLayers.data();
    }
    VK_CHECK(vkCreateInstance(&CI, this->Allocator,&VulkanInstanceHandle));
    
#ifndef NDEBUG
    if(enableValidationLayer){
      VkDebugUtilsMessengerCreateInfoEXT CI={};
      CI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      CI.pNext =nullptr;
      CI.pUserData = nullptr;
      CI.pfnUserCallback = debugCallback;
      CI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      CI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT| VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

      debugMessenger = CreateDebugUtilsMessenger(CI);
      if(debugMessenger == VK_NULL_HANDLE){
        std::cout<<"Create Debug Uitls messenger failed\n";
        exit(-1);
      }
    }
#endif
  }

  operator const VkInstance&()const{
    return VulkanInstanceHandle;
  }

  operator VkInstance & (){
    return VulkanInstanceHandle;
  }

  VkInstance* operator&(){
    return &VulkanInstanceHandle;
  }

  ~VkInstanceObject(){
    DestroyDebugUtilsMessenger(debugMessenger);
    vkDestroyInstance(VulkanInstanceHandle, Allocator);
  }

  VkAllocationCallbacks * Allocator = nullptr;
  VkInstance VulkanInstanceHandle=VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;


  static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
      VkDebugUtilsMessageSeverityFlagBitsEXT serverity,
      VkDebugUtilsMessageTypeFlagsEXT msgType,
      const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
      void *pUserData)

  {
    using namespace std;
    (void)serverity;
    (void)msgType;
    (void)pUserData;
    cerr<<"Validation layer: "<<pCallbackData->pMessage<<endl;
    return VK_FALSE;
  }

  VkDebugUtilsMessengerEXT CreateDebugUtilsMessenger(const VkDebugUtilsMessengerCreateInfoEXT &CI){
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(VulkanInstanceHandle,"vkCreateDebugUtilsMessengerEXT");
    VkDebugUtilsMessengerEXT vkHandle = VK_NULL_HANDLE;
    if(func){
      VK_CHECK(func(VulkanInstanceHandle,&CI,Allocator,&vkHandle));
    }else{
      std::cout<<"Failed to load vkCreateDebugUtilsMessengerEXT";
      exit(-1);
    }
    return vkHandle;
  }
  void DestroyDebugUtilsMessenger(VkDebugUtilsMessengerEXT object){
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(VulkanInstanceHandle,"vkDestroyDebugUtilsMessengerEXT");
    if(func){
      func(VulkanInstanceHandle,object,Allocator);
    }else{
      std::cout<<"Failed to load vkDestroyDebugUtilsMessengerEXT";
      exit(-1);
    }
  }
     
};

struct VkPhysicalDeviceObject{
  std::shared_ptr<VkInstanceObject> Instance;
  std::vector<VkMemoryType> AllowedMemoryType;
  std::vector<VkMemoryHeap> AllowedHeaps;
  VkPhysicalDeviceFeatures PhysicalDeviceFeatures;
  std::vector<VkQueueFamilyProperties> QueueFamilies;


  VkPhysicalDeviceObject(std::shared_ptr<VkInstanceObject> instance):Instance(std::move(instance)){
    using namespace std;
    uint32_t count = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(Instance->VulkanInstanceHandle, &count,nullptr));
    vector<VkPhysicalDevice> devs(count);
    VK_CHECK(vkEnumeratePhysicalDevices(Instance->VulkanInstanceHandle, &count,devs.data()));

    cout<<"Number of physical device: "<<devs.size()<<endl;

    for(int i=0;i<devs.size();i++){
      VkPhysicalDeviceProperties prop;
      auto d = devs[i];
      vkGetPhysicalDeviceProperties(d, &prop);

      VkPhysicalDeviceMemoryProperties memProp;
      vkGetPhysicalDeviceMemoryProperties(d,&memProp);

      AllowedMemoryType = vector<VkMemoryType>(memProp.memoryTypes,memProp.memoryTypes + memProp.memoryTypeCount);
      cout<<"Memory Type:"<<AllowedMemoryType.size()<<endl;
      AllowedHeaps = vector<VkMemoryHeap>(memProp.memoryHeaps,memProp.memoryHeaps + memProp.memoryHeapCount);
      cout<<"Memory Heap:"<<AllowedHeaps.size()<<endl;

      vkGetPhysicalDeviceFeatures(d,&PhysicalDeviceFeatures);

      uint32_t queCount = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(d,&queCount,nullptr);
      QueueFamilies.resize(queCount);
      vkGetPhysicalDeviceQueueFamilyProperties(d, &queCount,QueueFamilies.data());

      PhysicalDevice = d;
      break;

    }

  }

  const VkPhysicalDeviceFeatures & GetPhysicalDeviceFeatures()const{
    return PhysicalDeviceFeatures;
  }

  int GetQueueIndex(VkQueueFlagBits queueFlags)const{
    const auto count = QueueFamilies.size();
    for(int i = 0 ; i< count;i++){
      const auto &q = QueueFamilies[i];
      if (q.queueCount > 0 && q.queueFlags & queueFlags){
        return i;
      }
    }
    std::cout<<"specified queue is not found";
    exit(-1);
    return -1;
  }
  ~VkPhysicalDeviceObject()= default;

  VkPhysicalDeviceObject(const VkPhysicalDeviceObject & ) = delete;
  VkPhysicalDeviceObject & operator=(const VkPhysicalDeviceObject &) = delete;
  VkPhysicalDeviceObject(VkPhysicalDeviceObject &&)noexcept = default;
  VkPhysicalDeviceObject & operator=(VkPhysicalDeviceObject &&)noexcept = default;

  operator const VkPhysicalDevice&()const{
    return PhysicalDevice;
  }

  operator VkPhysicalDevice&(){
    return PhysicalDevice;
  }

  VkPhysicalDevice* operator&(){
    return &PhysicalDevice;
  }

  private:
  VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
  
};


