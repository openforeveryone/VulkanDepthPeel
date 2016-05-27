/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

//BEGIN_INCLUDE(all)
#include <initializer_list>
#include <jni.h>
#include <errno.h>
#include <cassert>
#include <unistd.h>


//#include <EGL/egl.h>
//#include <GLES/gl.h>

#define VK_USE_PLATFORM_ANDROID_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vk_platform.h>
#include <stdio.h>
#include <math.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "native-activity", __VA_ARGS__))

struct Vertex{
    float posX, posY, posZ, posW; // Position data
    float r, g, b, a;             // Color
};

#define XYZ1(_x_, _y_, _z_) (_x_), (_y_), (_z_), 1.f
#define XYZp5(_x_, _y_, _z_) (_x_), (_y_), (_z_), 0.1f

static const struct Vertex vertexData[] = {
        {XYZ1(-1, -1, -1), XYZp5(0.f, 0.f, 0.f)},
        {XYZ1(1, -1, -1), XYZp5(1.f, 0.f, 0.f)},
        {XYZ1(-1, 1, -1), XYZp5(0.f, 1.f, 0.f)},
        {XYZ1(-1, 1, -1), XYZp5(0.f, 1.f, 0.f)},
        {XYZ1(1, -1, -1), XYZp5(1.f, 0.f, 0.f)},
        {XYZ1(1, 1, -1), XYZp5(1.f, 1.f, 0.f)},

        {XYZ1(-1, -1, 1), XYZp5(0.f, 0.f, 1.f)},
        {XYZ1(-1, 1, 1), XYZp5(0.f, 1.f, 1.f)},
        {XYZ1(1, -1, 1), XYZp5(1.f, 0.f, 1.f)},
        {XYZ1(1, -1, 1), XYZp5(1.f, 0.f, 1.f)},
        {XYZ1(-1, 1, 1), XYZp5(0.f, 1.f, 1.f)},
        {XYZ1(1, 1, 1), XYZp5(1.f, 1.f, 1.f)},

        {XYZ1(1, 1, 1), XYZp5(1.f, 1.f, 1.f)},
        {XYZ1(1, 1, -1), XYZp5(1.f, 1.f, 0.f)},
        {XYZ1(1, -1, 1), XYZp5(1.f, 0.f, 1.f)},
        {XYZ1(1, -1, 1), XYZp5(1.f, 0.f, 1.f)},
        {XYZ1(1, 1, -1), XYZp5(1.f, 1.f, 0.f)},
        {XYZ1(1, -1, -1), XYZp5(1.f, 0.f, 0.f)},

        {XYZ1(-1, 1, 1), XYZp5(0.f, 1.f, 1.f)},
        {XYZ1(-1, -1, 1), XYZp5(0.f, 0.f, 1.f)},
        {XYZ1(-1, 1, -1), XYZp5(0.f, 1.f, 0.f)},
        {XYZ1(-1, 1, -1), XYZp5(0.f, 1.f, 0.f)},
        {XYZ1(-1, -1, 1), XYZp5(0.f, 0.f, 1.f)},
        {XYZ1(-1, -1, -1), XYZp5(0.f, 0.f, 0.f)},

        {XYZ1(1, 1, 1), XYZp5(1.f, 1.f, 1.f)},
        {XYZ1(-1, 1, 1), XYZp5(0.f, 1.f, 1.f)},
        {XYZ1(1, 1, -1), XYZp5(1.f, 1.f, 0.f)},
        {XYZ1(1, 1, -1), XYZp5(1.f, 1.f, 0.f)},
        {XYZ1(-1, 1, 1), XYZp5(0.f, 1.f, 1.f)},
        {XYZ1(-1, 1, -1), XYZp5(0.f, 1.f, 0.f)},

        {XYZ1(1, -1, 1), XYZp5(1.f, 0.f, 1.f)},
        {XYZ1(1, -1, -1), XYZp5(1.f, 0.f, 0.f)},
        {XYZ1(-1, -1, 1), XYZp5(0.f, 0.f, 1.f)},
        {XYZ1(-1, -1, 1), XYZp5(0.f, 0.f, 1.f)},
        {XYZ1(1, -1, -1), XYZp5(1.f, 0.f, 0.f)},
        {XYZ1(-1, -1, -1), XYZp5(0.f, 0.f, 0.f)},
};

/*
 * Simulates desktop's glRotatef. The matrix is returned in column-major
 * order.
 */
void rotate_matrix(double angle, double x, double y, double z, float *R) {
    double radians, c, s, c1, u[3], length;
    int i, j;

    radians = (angle * M_PI) / 180.0;

    c = cos(radians);
    s = sin(radians);

    c1 = 1.0 - cos(radians);

    length = sqrt(x * x + y * y + z * z);

    u[0] = x / length;
    u[1] = y / length;
    u[2] = z / length;

    for (i = 0; i < 16; i++) {
        R[i] = 0.0;
    }

    R[15] = 1.0;

    for (i = 0; i < 3; i++) {
        R[i * 4 + (i + 1) % 3] = u[(i + 2) % 3] * s;
        R[i * 4 + (i + 2) % 3] = -u[(i + 1) % 3] * s;
    }

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            R[i * 4 + j] += c1 * u[i] * u[j] + (i == j ? c : 0.0);
        }
    }
}

/*
 * Simulates gluPerspectiveMatrix
 */
void perspective_matrix(double fovy, double aspect, double znear, double zfar, float *P) {
    int i;
    double f;

    f = 1.0/tan(fovy * 0.5);

    for (i = 0; i < 16; i++) {
        P[i] = 0.0;
    }

    P[0] = f / aspect;
    P[5] = f;
    P[10] = (znear + zfar) / (znear - zfar);
    P[11] = -1.0;
    P[14] = (2.0 * znear * zfar) / (znear - zfar);
    P[15] = 0.0;
}
/*
 * Multiplies A by B and writes out to C. All matrices are 4x4 and column
 * major. In-place multiplication is supported.
 */
void multiply_matrix(float *A, float *B, float *C) {
    int i, j, k;
    float aTmp[16];

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            aTmp[j * 4 + i] = 0.0;

            for (k = 0; k < 4; k++) {
                aTmp[j * 4 + i] += A[k * 4 + i] * B[j * 4 + k];
            }
        }
    }

    memcpy(C, aTmp, sizeof(aTmp));
}

void identity_matrix(float *matrix) {
    float aTmp[16]={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    memcpy(matrix, aTmp, sizeof(aTmp));
}

void translate_matrix(double x, double y, double z, float *matrix) {
    identity_matrix(matrix);
    matrix[12]=x;
    matrix[13]=y;
    matrix[14]=z;
}

/**
 * Our saved state data.
 */
struct saved_state {
    float angle;
    int32_t x;
    int32_t y;
};

/**
 * Shared state for our app.
 */
struct engine {
    struct android_app* app;

    ASensorManager* sensorManager;
    const ASensor* accelerometerSensor;
    ASensorEventQueue* sensorEventQueue;

    int animating = 1;
    VkInstance vkInstance;
    VkDevice vkDevice;
    VkPhysicalDevice physicalDevice;
    VkCommandBuffer setupCommandBuffer;
    VkCommandBuffer renderCommandBuffer[2];
    VkImage depthImage;
    VkImageView depthView;
    VkDeviceMemory depthMemory;
    uint32_t swapchainImageCount = 0;
    VkSwapchainKHR swapchain;
    VkImage *swapChainImages;
    VkImageView *swapChainViews;
    VkFramebuffer *framebuffers;
    uint8_t *uniformMappedMemory;
    VkSemaphore presentCompleteSemaphore;
    VkRenderPass renderPass[2];
    VkPipelineLayout pipelineLayout;
    VkDescriptorSet *descriptorSets;
    VkBuffer vertexBuffer;
    VkQueue queue;
    bool vulkanSetupOK;
    int frame = 0;
    int32_t width;
    int32_t height;
    struct saved_state state;
    VkPipeline pipeline[2];

    const int NUM_SAMPLES = 1;
};


char* loadAsset(const char* filename, struct engine *pEngine, bool &ok, size_t &size)
{
    ok=false;
    char *buffer = NULL;
    AAsset* asset = AAssetManager_open(pEngine->app->activity->assetManager, filename, AASSET_MODE_STREAMING);
    if (!asset) {
        LOGE("Cannot open asset %s", filename);
        return NULL;
    }
    size = AAsset_getLength(asset);
    if (size==0)
    {
        LOGE("Cannot open asset %s (file empty)", filename);
        return NULL;
    }
    buffer = (char*)malloc(size);
    int bytesRead = AAsset_read(asset, buffer, size);
    if (bytesRead < 0) {
        LOGE("Cannot read asset %s", filename);
        return NULL;
    }
    AAsset_close(asset);
    ok=true;
    LOGI("File %s read %d bytes.", filename, bytesRead);
    return buffer;
}

static int pfd[2];
static pthread_t thr;

static void *std_log_thread_main(void*)
{
    //This function is from https://codelab.wordpress.com/2014/11/03/how-to-use-standard-output-streams-for-logging-in-android-apps/
    ssize_t rdsz;
    char buf[128];
    while((rdsz = read(pfd[0], buf, sizeof buf - 1)) > 0) {
        if(buf[rdsz - 1] == '\n') --rdsz;
        buf[rdsz - 1] = 0;  /* add null-terminator */
        __android_log_write(ANDROID_LOG_DEBUG, "native-activity-std", buf);
    }
    return 0;
}

/**
 * Initialize an EGL context for the current display.
 */
static int engine_init_display(struct engine* engine) {
    // initialize Vulkan

    LOGI ("Initializing Vulkan\n");

    //Redirect stdio to android log so vulkan validation layer output is not lost.
    //Redirect code from https://codelab.wordpress.com/2014/11/03/how-to-use-standard-output-streams-for-logging-in-android-apps/
    /* make stdout line-buffered and stderr unbuffered */
    setvbuf(stdout, 0, _IOLBF, 0);
    setvbuf(stderr, 0, _IONBF, 0);

    /* create the pipe and redirect stdout and stderr */
    pipe(pfd);
    dup2(pfd[1], 1);
    dup2(pfd[1], 2);

    /* spawn the logging thread */
    if(pthread_create(&thr, 0, std_log_thread_main, 0) == -1)
        return -1;
    pthread_detach(thr);

    printf("Testing 123");

    //Set the working directory to somewhere that the

    char oldcwd[1024];
    char cwd[1024];
    if (getcwd(oldcwd, sizeof(oldcwd)) != NULL)
        LOGI("Current working dir: %s\n", oldcwd);

    LOGI("internalDataPath: %s\n", engine->app->activity->internalDataPath);

    chdir(engine->app->activity->internalDataPath);

    if (getcwd(cwd, sizeof(cwd)) != NULL)
        LOGI("Current working dir: %s\n", cwd);

    //We will put the working directory back to oldcwd later.

    VkResult res;

    uint32_t availableLayerCount =0;
    res = vkEnumerateInstanceLayerProperties(&availableLayerCount, NULL);
    LOGI("There are %d instance layers avalible\n", availableLayerCount);
    VkLayerProperties availableLayers[availableLayerCount];
    res = vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers);
    for (int i =0; i < availableLayerCount; i++)
    {
        LOGI("%s: %s\n", availableLayers[i].layerName, availableLayers[i].description);
    }

    const char *enabledLayerNames[] = {
            //List any layers you want to enable here.
        "VK_LAYER_LUNARG_core_validation"
    };

    const char *enabledInstanceExtensionNames[] = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
    };

    VkApplicationInfo app_info;
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = NULL;
    app_info.pApplicationName = "My Test App";
    app_info.applicationVersion = 1;
    app_info.pEngineName = "My Test App Engine";
    app_info.engineVersion = 1;
    app_info.apiVersion = VK_MAKE_VERSION(1, 0, 3);

    //Initialize the VkInstanceCreateInfo structure
    VkInstanceCreateInfo inst_info;
    inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    inst_info.pNext = NULL;
    inst_info.flags = 0;
    inst_info.pApplicationInfo = &app_info;
    inst_info.enabledExtensionCount = 2;
    inst_info.ppEnabledExtensionNames = enabledInstanceExtensionNames;
    inst_info.enabledLayerCount = 1;
    inst_info.ppEnabledLayerNames = enabledLayerNames;

    res = vkCreateInstance(&inst_info, NULL, &engine->vkInstance);
    if (res == VK_ERROR_INCOMPATIBLE_DRIVER) {
        LOGE ("vkCreateInstance returned VK_ERROR_INCOMPATIBLE_DRIVER\n");
        return -1;
    } else if (res != VK_SUCCESS) {
        LOGE ("vkCreateInstance returned error %d\n", res);
        return -1;
    }
    LOGI ("Vulkan instance created\n");

    VkAndroidSurfaceCreateInfoKHR instInfo;
    instInfo.sType=VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    instInfo.pNext=NULL;
    instInfo.window=engine->app->window;

    VkSurfaceKHR surface;
    res =  vkCreateAndroidSurfaceKHR(engine->vkInstance, &instInfo, NULL, &surface);
    if (res != VK_SUCCESS) {
        LOGE ("vkCreateAndroidSurfaceKHR returned error.\n");
        return -1;
    }
    LOGI ("Vulkan surface created\n");

    uint32_t deviceBufferSize=0;
    res = vkEnumeratePhysicalDevices(engine->vkInstance, &deviceBufferSize, NULL);
    LOGI ("GPU Count: %i\n", deviceBufferSize);
    VkPhysicalDevice physicalDevices[deviceBufferSize];
    res = vkEnumeratePhysicalDevices(engine->vkInstance, &deviceBufferSize, physicalDevices);
    if (res == VK_ERROR_INITIALIZATION_FAILED) {
        LOGE ("vkEnumeratePhysicalDevices returned VK_ERROR_INITIALIZATION_FAILED for GPU 0.\n");
        return -1;
    }else if (res != VK_SUCCESS) {
        LOGE ("vkEnumeratePhysicalDevices returned error.\n");
        return -1;
    }
    engine->physicalDevice=physicalDevices[0];

    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(engine->physicalDevice, &physicalDeviceMemoryProperties);
    LOGI ("There are %d memory types.\n", physicalDeviceMemoryProperties.memoryTypeCount);

    VkDeviceQueueCreateInfo deviceQueueCreateInfo;
    deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo.pNext = NULL;
    deviceQueueCreateInfo.queueCount = 1;
    float queuePriorities[1] = {1.0};
    deviceQueueCreateInfo.pQueuePriorities = queuePriorities;

    uint32_t queueCount=0;
    //We are only using the first physical device:
    vkGetPhysicalDeviceQueueFamilyProperties(engine->physicalDevice, &queueCount, NULL);
    LOGI ("%i PhysicalDeviceQueueFamily(ies).\n", queueCount);

    VkQueueFamilyProperties queueFamilyProperties[queueCount];
    vkGetPhysicalDeviceQueueFamilyProperties(engine->physicalDevice, &queueCount, queueFamilyProperties);
    int found = 0;
    unsigned int i = 0;
    VkBool32 supportsPresent;
    for (; i < queueCount; i++) {
        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            LOGI ("PhysicalDeviceQueueFamily %i has property VK_QUEUE_GRAPHICS_BIT.\n", i);
            vkGetPhysicalDeviceSurfaceSupportKHR(engine->physicalDevice, i, surface, &supportsPresent);
            if (supportsPresent) {
                deviceQueueCreateInfo.queueFamilyIndex = i;
                found = 1;
                break;
            }
        }
    }
    if (found==0) {
        LOGE ("Error: A suitable queue family has not been found.\n");
        return -1;
    }

    availableLayerCount =0;
    res = vkEnumerateDeviceLayerProperties(engine->physicalDevice, &availableLayerCount, NULL);
    LOGI("There are %d device layers avalible\n", availableLayerCount);
    availableLayers[availableLayerCount];
    res = vkEnumerateDeviceLayerProperties(engine->physicalDevice, &availableLayerCount,
                                           availableLayers);
    for (int i =0; i < availableLayerCount; i++)
    {
        LOGI("%s: %s\n", availableLayers[i].layerName, availableLayers[i].description);
    }

    const char *enabledDeviceExtensionNames[] = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceCreateInfo dci = {};
    dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dci.pNext = NULL;
    dci.queueCreateInfoCount = 1;
    dci.pQueueCreateInfos = &deviceQueueCreateInfo;
    dci.enabledLayerCount = 0;
    dci.ppEnabledLayerNames = NULL;
    dci.enabledExtensionCount = 1;
    dci.ppEnabledExtensionNames = enabledDeviceExtensionNames;
    dci.pEnabledFeatures = NULL;
    dci.enabledLayerCount = 1;
    dci.ppEnabledLayerNames = enabledLayerNames;

    res = vkCreateDevice(engine->physicalDevice, &dci, NULL, &engine->vkDevice);
    if (res == VK_ERROR_INITIALIZATION_FAILED) {
        LOGE ("vkCreateDevice returned VK_ERROR_INITIALIZATION_FAILED for GPU 0.\n");
        return -1;
    }else if (res != VK_SUCCESS) {
        LOGE ("vkCreateDevice returned error %d.\n", res);
        return -1;
    }
    LOGI("vkCreateDevice successful");


    //Setup the swapchain
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(engine->physicalDevice, surface, &formatCount, NULL);
    VkSurfaceFormatKHR formats[formatCount];
    vkGetPhysicalDeviceSurfaceFormatsKHR(engine->physicalDevice, surface, &formatCount, formats);

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(engine->physicalDevice, surface, &presentModeCount, NULL);
    VkPresentModeKHR presentModes[presentModeCount];
    vkGetPhysicalDeviceSurfacePresentModesKHR(engine->physicalDevice, surface, &presentModeCount, presentModes);

    VkSurfaceCapabilitiesKHR surfCapabilities;
    res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(engine->physicalDevice, surface, &surfCapabilities);

    VkExtent2D swapChainExtent;
    // width and height are either both -1, or both not -1.
    if (surfCapabilities.currentExtent.width == (uint32_t)-1) {
        // If the surface size is undefined, the size is set to
        // the size of the images requested.
//        swapChainExtent.width = 800;
//        swapChainExtent.height = 600;
        LOGE("Swapchain size is (-1, -1)\n");
        return -1;
    } else {
        // If the surface size is defined, the swap chain size must match
        swapChainExtent = surfCapabilities.currentExtent;
        LOGI("Swapchain size is (%d, %d)\n", swapChainExtent.width, swapChainExtent.height);
        engine->width=swapChainExtent.width;
        engine->height=swapChainExtent.height;
    }

    // If the format list includes just one entry of VK_FORMAT_UNDEFINED,
    // the surface has no preferred format.  Otherwise, at least one
    // supported format will be returned.
    VkFormat format;
    if (formatCount == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
        format = VK_FORMAT_R8G8B8A8_UNORM;
    } else {
        assert(formatCount >= 1);
        format = formats[0].format;
    }
    LOGI("Using format %d\n", format);

    uint32_t desiredNumberOfSwapChainImages = surfCapabilities.minImageCount + 1;
    if ((surfCapabilities.maxImageCount > 0) &&
        (desiredNumberOfSwapChainImages > surfCapabilities.maxImageCount)) {
        // Application must settle for fewer images than desired:
        desiredNumberOfSwapChainImages = surfCapabilities.maxImageCount;
    }
    LOGI("Asking for %d SwapChainImages\n", desiredNumberOfSwapChainImages);

    VkSurfaceTransformFlagBitsKHR preTransform;
    if (surfCapabilities.supportedTransforms &
        VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        preTransform = surfCapabilities.currentTransform;
    }
    LOGI("Using preTransform %d\n", preTransform);

    VkSwapchainCreateInfoKHR swapCreateInfo;
    swapCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapCreateInfo.pNext = NULL;
    swapCreateInfo.surface = surface;
    swapCreateInfo.minImageCount = desiredNumberOfSwapChainImages;
    swapCreateInfo.imageFormat = format;
    swapCreateInfo.imageExtent=swapChainExtent;
    //swapCreateInfo.imageExtent.width = width; //Should match window size
    //swapCreateInfo.imageExtent.height = height;
    swapCreateInfo.preTransform = preTransform;
    swapCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapCreateInfo.imageArrayLayers = 1;
    swapCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapCreateInfo.oldSwapchain = VK_NULL_HANDLE;
    swapCreateInfo.clipped = VK_TRUE;
    swapCreateInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    swapCreateInfo.imageUsage =
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    swapCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapCreateInfo.queueFamilyIndexCount = 0;
    swapCreateInfo.pQueueFamilyIndices = NULL;

    vkCreateSwapchainKHR(engine->vkDevice, &swapCreateInfo, NULL, &engine->swapchain);
    if (res != VK_SUCCESS) {
        LOGE ("vkCreateSwapchainKHR returned error.\n");
        return -1;
    }
    LOGI("Swapchain created");

    //Setup Command buffers
    VkCommandPool commandPool;
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.pNext = NULL;
    commandPoolCreateInfo.queueFamilyIndex = deviceQueueCreateInfo.queueFamilyIndex;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    res = vkCreateCommandPool(engine->vkDevice, &commandPoolCreateInfo, NULL, &commandPool);
    if (res != VK_SUCCESS) {
        LOGE ("vkCreateCommandPool returned error.\n");
        return -1;
    }

    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.pNext = NULL;
    commandBufferAllocateInfo.commandPool = commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 3;

    VkCommandBuffer commandBuffers[3];
    res = vkAllocateCommandBuffers(engine->vkDevice, &commandBufferAllocateInfo, commandBuffers);
    if (res != VK_SUCCESS) {
        LOGE ("vkAllocateCommandBuffers returned error.\n");
        return -1;
    }
    engine->setupCommandBuffer=commandBuffers[0];
    engine->renderCommandBuffer[0]=commandBuffers[1];
    engine->renderCommandBuffer[1]=commandBuffers[2];

    LOGI("Command buffers created");

    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = NULL;
    commandBufferBeginInfo.flags = 0;
    commandBufferBeginInfo.pInheritanceInfo = NULL;

    res = vkBeginCommandBuffer(engine->setupCommandBuffer, &commandBufferBeginInfo);
    if (res != VK_SUCCESS) {
        LOGE ("vkBeginCommandBuffer returned error.\n");
        return -1;
    }

    vkGetDeviceQueue(engine->vkDevice, deviceQueueCreateInfo.queueFamilyIndex, 0, &engine->queue);

    vkGetSwapchainImagesKHR(engine->vkDevice, engine->swapchain, &engine->swapchainImageCount, NULL);
    engine->swapChainImages = new VkImage[engine->swapchainImageCount];
    res = vkGetSwapchainImagesKHR(engine->vkDevice, engine->swapchain, &engine->swapchainImageCount, engine->swapChainImages);
    if (res != VK_SUCCESS) {
        LOGE ("vkCreateSwapchainKHR returned error.\n");
        return -1;
    }
    printf ("swapchainImageCount %d.\n",engine->swapchainImageCount);

    engine->swapChainViews=(VkImageView*)malloc(sizeof (VkImageView) *engine->swapchainImageCount);
    for (uint32_t i = 0; i < engine->swapchainImageCount; i++) {
        LOGI ("Setting up swapChainView %d.\n",i);
        VkImageViewCreateInfo color_image_view = {};
        color_image_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        color_image_view.pNext = NULL;
        color_image_view.format = format;
        color_image_view.components.r = VK_COMPONENT_SWIZZLE_R;
        color_image_view.components.g = VK_COMPONENT_SWIZZLE_G;
        color_image_view.components.b = VK_COMPONENT_SWIZZLE_B;
        color_image_view.components.a = VK_COMPONENT_SWIZZLE_A;
        color_image_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        color_image_view.subresourceRange.baseMipLevel = 0;
        color_image_view.subresourceRange.levelCount = 1;
        color_image_view.subresourceRange.baseArrayLayer = 0;
        color_image_view.subresourceRange.layerCount = 1;
        color_image_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
        color_image_view.flags = 0;
        color_image_view.image = engine->swapChainImages[i];

        VkImageMemoryBarrier imageMemoryBarrier;
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        imageMemoryBarrier.pNext = NULL;
        imageMemoryBarrier.image = engine->swapChainImages[i];
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
        imageMemoryBarrier.subresourceRange.levelCount = 1;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
        imageMemoryBarrier.subresourceRange.layerCount = 1;
        imageMemoryBarrier.srcQueueFamilyIndex=0;
        imageMemoryBarrier.dstQueueFamilyIndex=0;
        //imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
        //imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        //imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = 0;

        // Put barrier on top
        VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkPipelineStageFlags destStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

        // Put barrier inside setup command buffer
        vkCmdPipelineBarrier(engine->setupCommandBuffer, srcStageFlags, destStageFlags, 0,
                             0, NULL, 0, NULL, 1, &imageMemoryBarrier);

        res = vkCreateImageView(engine->vkDevice, &color_image_view, NULL, &engine->swapChainViews[i]);
        if (res != VK_SUCCESS) {
            LOGE ("vkCreateImageView returned error.\n");
            return -1;
        }
    }
    LOGI ("swapchainImageCount %d.\n", engine->swapchainImageCount);

    //Setup the depth buffer:
    const VkFormat depth_format = VK_FORMAT_D16_UNORM;

    VkImageCreateInfo imageCreateInfo;
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(engine->physicalDevice, depth_format, &props);
    if (! (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
        LOGE ("depth_format %d Unsupported.\n", depth_format);
        return -1;
    }

    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext = NULL;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = depth_format;
    imageCreateInfo.extent.width = swapChainExtent.width;
    imageCreateInfo.extent.height = swapChainExtent.height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.queueFamilyIndexCount = 0;
    imageCreateInfo.pQueueFamilyIndices = NULL;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageCreateInfo.flags = 0;

    //Create image for depth buffer
    res = vkCreateImage(engine->vkDevice, &imageCreateInfo, NULL, &engine->depthImage);
    if (res != VK_SUCCESS) {
        LOGE ("vkCreateImage returned error while creating depth buffer.\n");
        return -1;
    }

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(engine->vkDevice, engine->depthImage, &memoryRequirements);

    uint32_t typeBits = memoryRequirements.memoryTypeBits;
    uint32_t typeIndex;
    //Get the index of the first set bit:
    for (typeIndex = 0; typeIndex < 32; typeIndex++) {
        if ((typeBits & 1) == 1)//Check last bit;
            break;
        typeBits >>= 1;
    }

    VkMemoryAllocateInfo memAllocInfo;
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.pNext = NULL;
    memAllocInfo.allocationSize = memoryRequirements.size;
    memAllocInfo.memoryTypeIndex = typeIndex;

    //Allocate memory
    res = vkAllocateMemory(engine->vkDevice, &memAllocInfo, NULL, &engine->depthMemory);
    if (res != VK_SUCCESS) {
        LOGE ("vkAllocateMemory returned error while creating depth buffer.\n");
        return -1;
    }

    //Bind memory
    res = vkBindImageMemory(engine->vkDevice, engine->depthImage, engine->depthMemory, 0);
    if (res != VK_SUCCESS) {
        LOGE ("vkBindImageMemory returned error while creating depth buffer. %d\n", res);
        return -1;
    }

    VkImageMemoryBarrier imageMemoryBarrier;
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    imageMemoryBarrier.pNext = NULL;
    imageMemoryBarrier.image = engine->depthImage;
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    imageMemoryBarrier.subresourceRange.levelCount = 1;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    imageMemoryBarrier.subresourceRange.layerCount = 1;
    imageMemoryBarrier.srcQueueFamilyIndex=0;
    imageMemoryBarrier.dstQueueFamilyIndex=0;
    imageMemoryBarrier.srcAccessMask = 0;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    // Put barrier on top
    VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags destStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    // Put barrier inside setup command buffer
    vkCmdPipelineBarrier(engine->setupCommandBuffer, srcStageFlags, destStageFlags, 0,
                         0, NULL, 0, NULL, 1, &imageMemoryBarrier);


    //Create image view
    VkImageViewCreateInfo view_info;
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.pNext = NULL;
    view_info.image = engine->depthImage;
    view_info.format = depth_format;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    view_info.components.r = VK_COMPONENT_SWIZZLE_R;
    view_info.components.g = VK_COMPONENT_SWIZZLE_G;
    view_info.components.b = VK_COMPONENT_SWIZZLE_B;
    view_info.components.a = VK_COMPONENT_SWIZZLE_A;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.flags = 0;

    res = vkCreateImageView(engine->vkDevice, &view_info, NULL, &engine->depthView);
    if (res != VK_SUCCESS) {
        LOGE ("vkCreateImageView returned error while creating depth buffer. %d\n", res);
        return -1;
    }
    LOGI("Depth buffer created");

    res = vkEndCommandBuffer(engine->setupCommandBuffer);
    if (res != VK_SUCCESS) {
        LOGE ("vkEndCommandBuffer returned error %d.\n", res);
        return -1;
    }

    //Submit the setup command buffer
    VkSubmitInfo submitInfo[1];
    submitInfo[0].pNext = NULL;
    submitInfo[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo[0].waitSemaphoreCount = 0;
    submitInfo[0].pWaitSemaphores = NULL;
    submitInfo[0].pWaitDstStageMask = NULL;
    submitInfo[0].commandBufferCount = 1;
    submitInfo[0].pCommandBuffers = &engine->setupCommandBuffer;
    submitInfo[0].signalSemaphoreCount = 0;
    submitInfo[0].pSignalSemaphores = NULL;

    res = vkQueueSubmit(engine->queue, 1, submitInfo, VK_NULL_HANDLE);
    if (res != VK_SUCCESS) {
        LOGE ("vkQueueSubmit returned error %d.\n", res);
        return -1;
    }

    res = vkQueueWaitIdle(engine->queue);
    if (res != VK_SUCCESS) {
        LOGE ("vkQueueWaitIdle returned error %d.\n", res);
        return -1;
    }

    //Setup the renderpass:
    VkAttachmentDescription attachments[2];
    attachments[0].format = format;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[0].flags = 0;
    attachments[1].format = depth_format;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachments[1].flags = 0;

    attachments[2].format = format;
    attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[2].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[2].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[2].flags = 0;
    attachments[3].format = depth_format;
    attachments[3].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[3].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachments[3].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachments[3].flags = 0;

    VkAttachmentReference color_reference;
    color_reference.attachment = 0;
    color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_reference;
    depth_reference.attachment = 1;
    depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.flags = 0;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_reference;
    subpass.pResolveAttachments = NULL;
    subpass.pDepthStencilAttachment = &depth_reference;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = NULL;

    VkRenderPassCreateInfo rp_info[2];
    rp_info[0].sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_info[0].pNext = NULL;
    rp_info[0].attachmentCount = 2;
    rp_info[0].pAttachments = attachments;
    rp_info[0].subpassCount = 1;
    rp_info[0].pSubpasses = &subpass;
    rp_info[0].dependencyCount = 0;
    rp_info[0].pDependencies = NULL;

    rp_info[1].sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_info[1].pNext = NULL;
    rp_info[1].attachmentCount = 2;
    rp_info[1].pAttachments = &attachments[2];
    rp_info[1].subpassCount = 1;
    rp_info[1].pSubpasses = &subpass;
    rp_info[1].dependencyCount = 0;
    rp_info[1].pDependencies = NULL;

    res = vkCreateRenderPass(engine->vkDevice, &rp_info[0], NULL, &engine->renderPass[0]);
    if (res != VK_SUCCESS) {
        LOGE ("vkCreateRenderPass returned error. %d\n", res);
        return -1;
    }

    res = vkCreateRenderPass(engine->vkDevice, &rp_info[1], NULL, &engine->renderPass[1]);
    if (res != VK_SUCCESS) {
        LOGE ("vkCreateRenderPass returned error. %d\n", res);
        return -1;
    }

    LOGI("Renderpass created");


    //Setup the pipeline
    VkDescriptorSetLayoutBinding layout_bindings[1];
    layout_bindings[0].binding = 0;
    layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout_bindings[0].descriptorCount = 1;
    layout_bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layout_bindings[0].pImmutableSamplers = NULL;

    //Next take layout bindings and use them to create a descriptor set layout
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.pNext = NULL;
    descriptorSetLayoutCreateInfo.bindingCount = 1;
    descriptorSetLayoutCreateInfo.pBindings = layout_bindings;

    VkDescriptorSetLayout descriptorSetLayout;
    res = vkCreateDescriptorSetLayout(engine->vkDevice, &descriptorSetLayoutCreateInfo, NULL, &descriptorSetLayout);
    if (res != VK_SUCCESS) {
        LOGE ("vkCreateDescriptorSetLayout returned error.\n");
        return -1;
    }

    //Now use the descriptor layout to create a pipeline layout
    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo;
    pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pPipelineLayoutCreateInfo.pNext = NULL;
    pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pPipelineLayoutCreateInfo.pPushConstantRanges = NULL;
    pPipelineLayoutCreateInfo.setLayoutCount = 1;
    pPipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;

    res = vkCreatePipelineLayout(engine->vkDevice, &pPipelineLayoutCreateInfo, NULL, &engine->pipelineLayout);
    if (res != VK_SUCCESS) {
        LOGE ("vkCreatePipelineLayout returned error.\n");
        return -1;
    }

    //load shaders
    size_t vertexShaderSize=0;
    bool ok;
    char *vertexShader = loadAsset("shaders/vert.spv", engine, ok, vertexShaderSize);
    size_t fragmentShaderSize=0;
    char *fragmentShader = loadAsset("shaders/frag.spv", engine, ok, fragmentShaderSize);
    if (vertexShaderSize==0 || fragmentShaderSize==0){
        LOGE ("Colud not load shader file.\n");
        return -1;
    }


    VkShaderModuleCreateInfo moduleCreateInfo;
    VkPipelineShaderStageCreateInfo shaderStages[2];

    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].pNext = NULL;
    shaderStages[0].pSpecializationInfo = NULL;
    shaderStages[0].flags = 0;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].pName = "main";

    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = NULL;
    moduleCreateInfo.flags = 0;
    moduleCreateInfo.codeSize = vertexShaderSize;
    moduleCreateInfo.pCode = (uint32_t*)vertexShader; //This may not work with big-endian systems.
    res = vkCreateShaderModule(engine->vkDevice, &moduleCreateInfo, NULL, &shaderStages[0].module);
    if (res != VK_SUCCESS) {
        LOGE ("vkCreateShaderModule returned error %d.\n", res);
        return -1;
    }

    shaderStages[1].sType =  VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].pNext = NULL;
    shaderStages[1].pSpecializationInfo = NULL;
    shaderStages[1].flags = 0;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].pName = "main";

    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = NULL;
    moduleCreateInfo.flags = 0;
    moduleCreateInfo.codeSize = fragmentShaderSize;
    moduleCreateInfo.pCode = (uint32_t*)fragmentShader;
    res = vkCreateShaderModule(engine->vkDevice, &moduleCreateInfo, NULL, &shaderStages[1].module);
    if (res != VK_SUCCESS) {
        LOGE ("vkCreateShaderModule returned error %d.\n", res);
        return -1;
    }
    LOGI("Shaders Loaded");

    //Create the framebuffers
    engine->framebuffers=new VkFramebuffer[engine->swapchainImageCount];

    for (i = 0; i < engine->swapchainImageCount; i++) {

        VkImageView imageViewAttachments[2];

        //Attach the correct swapchain colourbuffer
        imageViewAttachments[0] = engine->swapChainViews[i];
        //We only have one depth buffer which we attach to all framebuffers
        imageViewAttachments[1] = engine->depthView;

        VkFramebufferCreateInfo fb_info;
        fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fb_info.pNext = NULL;
        fb_info.renderPass = engine->renderPass[0];
        fb_info.attachmentCount = 2;
        fb_info.pAttachments = imageViewAttachments;
        fb_info.width = swapChainExtent.width;
        fb_info.height = swapChainExtent.height;
        fb_info.layers = 1;

        res = vkCreateFramebuffer(engine->vkDevice, &fb_info, NULL, &engine->framebuffers[i]);
        if (res != VK_SUCCESS) {
            LOGE ("vkCreateFramebuffer returned error %d.\n", res);
            return -1;
        }
    }

    LOGI("%d framebuffers created", engine->swapchainImageCount);

    VkBufferCreateInfo uniformBufferCreateInfo;
    uniformBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    uniformBufferCreateInfo.pNext = NULL;
    uniformBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    uniformBufferCreateInfo.size = sizeof(float)*16; //Enough to store 1 matrix.
    uniformBufferCreateInfo.queueFamilyIndexCount = 0;
    uniformBufferCreateInfo.pQueueFamilyIndices = NULL;
    uniformBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    uniformBufferCreateInfo.flags = 0;

    VkBuffer uniformBuffer;
    res = vkCreateBuffer(engine->vkDevice, &uniformBufferCreateInfo, NULL, &uniformBuffer);
    if (res != VK_SUCCESS) {
        LOGE ("vkCreateBuffer returned error %d.\n", res);
        return -1;
    }

    vkGetBufferMemoryRequirements(engine->vkDevice, uniformBuffer, &memoryRequirements);

    typeBits = memoryRequirements.memoryTypeBits;
    VkFlags requirements_mask = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;// | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    for (typeIndex = 0; typeIndex < physicalDeviceMemoryProperties.memoryTypeCount; typeIndex++) {
        if ((typeBits & 1) == 1)//Check last bit;
        {
            if ((physicalDeviceMemoryProperties.memoryTypes[typeIndex].propertyFlags & requirements_mask) == requirements_mask)
            {
                found=1;
                break;
            }
            typeBits >>= 1;
        }
    }

    if (!found)
    {
        LOGE ("Did not find a suitible memory type.\n");
        return -1;
    }else
    LOGI ("Using memory type %d.\n", typeIndex);

    memAllocInfo.pNext = NULL;
    memAllocInfo.allocationSize = memoryRequirements.size;
    memAllocInfo.memoryTypeIndex = typeIndex;
    //
    VkDeviceMemory uniformMemory;
    res = vkAllocateMemory(engine->vkDevice, &memAllocInfo, NULL, &uniformMemory);
    if (res != VK_SUCCESS) {
        LOGE ("vkCreateBuffer returned error %d.\n", res);
        return -1;
    }

    res = vkMapMemory(engine->vkDevice, uniformMemory, 0, memoryRequirements.size, 0, (void **)&engine->uniformMappedMemory);
    if (res != VK_SUCCESS) {
        LOGE ("vkMapMemory returned error %d.\n", res);
        return -1;
    }

    res = vkBindBufferMemory(engine->vkDevice, uniformBuffer, uniformMemory, 0);
    if (res != VK_SUCCESS) {
        LOGE ("vkBindBufferMemory returned error %d.\n", res);
        return -1;
    }

    VkDescriptorBufferInfo uniformBufferInfo;
    uniformBufferInfo.buffer = uniformBuffer;
    uniformBufferInfo.offset = 0;
    uniformBufferInfo.range = sizeof(float)*16;

    //Create a descriptor pool
    VkDescriptorPoolSize typeCounts[1];
    typeCounts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    typeCounts[0].descriptorCount = 1;

    VkDescriptorPoolCreateInfo descriptorPoolInfo;
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.pNext = NULL;
    descriptorPoolInfo.maxSets = 1;
    descriptorPoolInfo.poolSizeCount = 1;
    descriptorPoolInfo.pPoolSizes = typeCounts;

    VkDescriptorPool descriptorPool;
    res = vkCreateDescriptorPool(engine->vkDevice, &descriptorPoolInfo, NULL, &descriptorPool);
    if (res != VK_SUCCESS) {
        printf ("vkCreateDescriptorPool returned error %d.\n", res);
        return -1;
    }

    //Create Vertex buffers:
    VkBufferCreateInfo vertexBufferCreateInfo;
    vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertexBufferCreateInfo.pNext = NULL;
    vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vertexBufferCreateInfo.size = sizeof(vertexData);
    vertexBufferCreateInfo.queueFamilyIndexCount = 0;
    vertexBufferCreateInfo.pQueueFamilyIndices = NULL;
    vertexBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vertexBufferCreateInfo.flags = 0;

    res = vkCreateBuffer(engine->vkDevice, &vertexBufferCreateInfo, NULL, &engine->vertexBuffer);
    if (res != VK_SUCCESS) {
        LOGE ("vkCreateDescriptorPool returned error %d.\n", res);
        return -1;
    }

    vkGetBufferMemoryRequirements(engine->vkDevice, engine->vertexBuffer, &memoryRequirements);
    typeBits = memoryRequirements.memoryTypeBits;
    requirements_mask = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    for (typeIndex = 0; typeIndex < physicalDeviceMemoryProperties.memoryTypeCount; typeIndex++) {
        if ((typeBits & 1) == 1)//Check last bit;
        {
            if ((physicalDeviceMemoryProperties.memoryTypes[typeIndex].propertyFlags & requirements_mask) == requirements_mask)
            {
                found=1;
                break;
            }
            typeBits >>= 1;
        }
    }

    if (!found)
    {
        LOGE ("Did not find a suitible memory type.\n");
        return -1;
    }else
        LOGI ("Using memory type %d.\n", typeIndex);

    memAllocInfo.pNext = NULL;
    memAllocInfo.allocationSize = memoryRequirements.size;
    memAllocInfo.memoryTypeIndex = typeIndex;

    VkDeviceMemory vertexMemory;
    res = vkAllocateMemory(engine->vkDevice, &memAllocInfo, NULL, &vertexMemory);
    if (res != VK_SUCCESS) {
        LOGE ("vkAllocateMemory returned error %d.\n", res);
        return -1;
    }

    uint8_t *vertexMappedMemory;
    res = vkMapMemory(engine->vkDevice, vertexMemory, 0, memoryRequirements.size, 0, (void **)&vertexMappedMemory);
    if (res != VK_SUCCESS) {
        LOGE ("vkMapMemory returned error %d.\n", res);
        return -1;
    }

    memcpy(vertexMappedMemory, vertexData, sizeof(vertexData));

    vkUnmapMemory(engine->vkDevice, vertexMemory);

    res = vkBindBufferMemory(engine->vkDevice, engine->vertexBuffer, vertexMemory, 0);
    if (res != VK_SUCCESS) {
        LOGE ("vkBindBufferMemory returned error %d.\n", res);
        return -1;
    }
    VkVertexInputBindingDescription vertexInputBindingDescription;
    vertexInputBindingDescription.binding = 0;
    vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vertexInputBindingDescription.stride = sizeof(vertexData[0]);

    VkVertexInputAttributeDescription vertexInputAttributeDescription[2];
    vertexInputAttributeDescription[0].binding = 0;
    vertexInputAttributeDescription[0].location = 0;
    vertexInputAttributeDescription[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    vertexInputAttributeDescription[0].offset = 0;
    vertexInputAttributeDescription[1].binding = 0;
    vertexInputAttributeDescription[1].location = 1;
    vertexInputAttributeDescription[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    vertexInputAttributeDescription[1].offset = 16;

    //Create a descriptor set
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo;
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.pNext = NULL;
    descriptorSetAllocateInfo.descriptorPool = descriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = 1;
    descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;


    engine->descriptorSets = new VkDescriptorSet[1];
    res = vkAllocateDescriptorSets(engine->vkDevice, &descriptorSetAllocateInfo, engine->descriptorSets);
    if (res != VK_SUCCESS) {
        printf ("vkAllocateDescriptorSets returned error %d.\n", res);
        return -1;
    }

    VkWriteDescriptorSet writes[1];
    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].pNext = NULL;
    writes[0].dstSet = engine->descriptorSets[0];
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[0].pBufferInfo = &uniformBufferInfo;
    writes[0].dstArrayElement = 0;
    writes[0].dstBinding = 0;

    vkUpdateDescriptorSets(engine->vkDevice, 1, writes, 0, NULL);


    VkViewport viewport;
    viewport.height = (float)swapChainExtent.height;
    viewport.width = (float)swapChainExtent.width;
    viewport.minDepth = (float)0.0f;
    viewport.maxDepth = (float)1.0f;
    viewport.x = 0;
    viewport.y = 0;

    VkRect2D scissor;
    scissor.extent.width = swapChainExtent.width;
    scissor.extent.height = swapChainExtent.height;
    scissor.offset.x = 0;
    scissor.offset.y = 0;

    //Create a pipeline object
    VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
    VkPipelineDynamicStateCreateInfo dynamicState;
    //No dynamic state:
    memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pNext = NULL;
    dynamicState.pDynamicStates = dynamicStateEnables;
    dynamicState.dynamicStateCount = 0;

    VkPipelineVertexInputStateCreateInfo vi;
    vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vi.pNext = NULL;
    vi.flags = 0;
    vi.vertexBindingDescriptionCount = 1;
    vi.pVertexBindingDescriptions = &vertexInputBindingDescription;
    vi.vertexAttributeDescriptionCount = 2;
    vi.pVertexAttributeDescriptions = vertexInputAttributeDescription;

    VkPipelineInputAssemblyStateCreateInfo ia;
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.pNext = NULL;
    ia.flags = 0;
    ia.primitiveRestartEnable = VK_FALSE;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineRasterizationStateCreateInfo rs;
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.pNext = NULL;
    rs.flags = 0;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_BACK_BIT;
    rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rs.depthClampEnable = VK_TRUE;
    rs.rasterizerDiscardEnable = VK_FALSE;
    rs.depthBiasEnable = VK_FALSE;
    rs.depthBiasConstantFactor = 0;
    rs.depthBiasClamp = 0;
    rs.depthBiasSlopeFactor = 0;
    rs.lineWidth = 1;

    VkPipelineColorBlendStateCreateInfo cb;
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb.flags = 0;
    cb.pNext = NULL;
    VkPipelineColorBlendAttachmentState att_state[1];
    att_state[0].colorWriteMask = 0xf;
    att_state[0].blendEnable = VK_TRUE;
    att_state[0].alphaBlendOp = VK_BLEND_OP_ADD;
    att_state[0].colorBlendOp = VK_BLEND_OP_ADD;
    att_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    att_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    att_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    cb.attachmentCount = 1;
    cb.pAttachments = att_state;
    cb.logicOpEnable = VK_FALSE;
    cb.logicOp = VK_LOGIC_OP_NO_OP;
    cb.blendConstants[0] = 1.0f;
    cb.blendConstants[1] = 1.0f;
    cb.blendConstants[2] = 1.0f;
    cb.blendConstants[3] = 1.0f;

    VkPipelineViewportStateCreateInfo vp = {};
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.pNext = NULL;
    vp.flags = 0;
    vp.viewportCount = 1;
//    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
    vp.scissorCount = 1;
//    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
    vp.pScissors = &scissor;
    vp.pViewports = &viewport;

    VkPipelineDepthStencilStateCreateInfo ds;
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.pNext = NULL;
    ds.flags = 0;
    ds.depthTestEnable = VK_TRUE;
    ds.depthWriteEnable = VK_TRUE;
    ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    ds.depthBoundsTestEnable = VK_FALSE;
    ds.stencilTestEnable = VK_FALSE;
    ds.minDepthBounds = 0;
    ds.maxDepthBounds = 0;
    ds.stencilTestEnable = VK_FALSE;
    ds.front = ds.back;

    VkPipelineMultisampleStateCreateInfo ms;
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.pNext = NULL;
    ms.flags = 0;
    ms.pSampleMask = NULL;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms.sampleShadingEnable = VK_FALSE;
    ms.alphaToCoverageEnable = VK_FALSE;
    ms.alphaToOneEnable = VK_FALSE;
    ms.minSampleShading = 0.0;

    VkGraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = NULL;
    pipelineInfo.layout = engine->pipelineLayout;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = 0;
    pipelineInfo.flags = 0;
    pipelineInfo.pVertexInputState = &vi;
    pipelineInfo.pInputAssemblyState = &ia;
    pipelineInfo.pRasterizationState = &rs;
    pipelineInfo.pColorBlendState = &cb;
    pipelineInfo.pTessellationState = NULL;
    pipelineInfo.pMultisampleState = &ms;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.pViewportState = &vp;
    pipelineInfo.pDepthStencilState = &ds;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.stageCount = 2;
    pipelineInfo.renderPass = engine->renderPass[0];
    pipelineInfo.subpass = 0;

    res = vkCreateGraphicsPipelines(engine->vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &engine->pipeline[0]);
    if (res != VK_SUCCESS) {
        printf ("vkCreateGraphicsPipelines re\turned error %d.\n", res);
        return -1;
    }

    pipelineInfo.renderPass = engine->renderPass[1];
    res = vkCreateGraphicsPipelines(engine->vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &engine->pipeline[1]);
    if (res != VK_SUCCESS) {
        printf ("vkCreateGraphicsPipelines re\turned error %d.\n", res);
        return -1;
    }

    VkSemaphoreCreateInfo presentCompleteSemaphoreCreateInfo;
    presentCompleteSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    presentCompleteSemaphoreCreateInfo.pNext = NULL;
    presentCompleteSemaphoreCreateInfo.flags = 0;

    res = vkCreateSemaphore(engine->vkDevice, &presentCompleteSemaphoreCreateInfo, NULL, &engine->presentCompleteSemaphore);
    if (res != VK_SUCCESS) {
        printf ("vkCreateSemaphore returned error.\n");
        return -1;
    }

    LOGI("Restoring working directory");
    chdir(oldcwd);

    if (getcwd(cwd, sizeof(cwd)) != NULL)
        LOGI("Current working dir: %s\n", cwd);

    engine->vulkanSetupOK=true;
    LOGI ("Vulkan setup complete");

    return 0;
}

void updateUniforms(struct engine* engine)
{
    //Create the uniforms
    float projectionMatrix[16];
    float viewMatrix[16];
    float modelMatrix[16]={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    float MVMatrix[16];
    float MVPMatrix[16];

    perspective_matrix(0.7853 /* 45deg */, (float)engine->width/(float)engine->height, 0.1f, 100.0f, projectionMatrix);
    translate_matrix(0,0,-5, viewMatrix);
    rotate_matrix(45+engine->frame, 0,1,0, modelMatrix);
    multiply_matrix(viewMatrix, modelMatrix, MVMatrix);
    //As the memory is still mapped we can write the result stright into uniformMappedMemory:
    multiply_matrix(projectionMatrix, MVMatrix, (float*)engine->uniformMappedMemory);
}

/**
 * Just the current frame in the display.
 */
static void engine_draw_frame(struct engine* engine) {

    if (!engine->vulkanSetupOK)
    {
//        LOGI("engine_draw_frame %d", engine->frame);
//        LOGI("Vulkan not ready");
        return;
    }

    VkClearValue clearValues[4];
    clearValues[0].color.float32[0] = 0.2f;
    clearValues[0].color.float32[1] = 0.2f;
    clearValues[0].color.float32[2] = 0.2f;
    clearValues[0].color.float32[3] = 0.2f;
    clearValues[1].depthStencil.depth = 1.0f;
    clearValues[1].depthStencil.stencil = 0;
    clearValues[2].color.float32[0] = 1.0f;
    clearValues[2].color.float32[1] = 0.0f;
    clearValues[2].color.float32[2] = 0.0f;
    clearValues[2].color.float32[3] = 0.0f;
    clearValues[3].depthStencil.depth = 1.0f;
    clearValues[3].depthStencil.stencil = 0;

    //The queue is idle, now is a good time to update the bound memory.
    updateUniforms(engine);

    uint32_t currentBuffer;
    VkResult res;

    // Get next image in the swap chain (back/front buffer)
    res = vkAcquireNextImageKHR(engine->vkDevice, engine->swapchain, UINT64_MAX, engine->presentCompleteSemaphore, NULL, &currentBuffer);
    if (res != VK_SUCCESS) {
        LOGE ("vkAcquireNextImageKHR returned error.\n");
        return;
    }

    for (int i = 0; i<2; i++) {
        VkRenderPassBeginInfo renderPassBeginInfo;
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.pNext = NULL;
        renderPassBeginInfo.renderPass = engine->renderPass[i];
        renderPassBeginInfo.framebuffer = engine->framebuffers[currentBuffer];
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width = engine->width;
        renderPassBeginInfo.renderArea.extent.height = engine->height;
        renderPassBeginInfo.clearValueCount = 2;
        renderPassBeginInfo.pClearValues = clearValues;// + (i*2);

        VkCommandBufferBeginInfo commandBufferBeginInfo = {};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.pNext = NULL;
        commandBufferBeginInfo.flags = (i == 0) ? 0 : VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        commandBufferBeginInfo.flags = 0;
        commandBufferBeginInfo.pInheritanceInfo = NULL;
        res = vkBeginCommandBuffer(engine->renderCommandBuffer[i], &commandBufferBeginInfo);
        if (res != VK_SUCCESS) {
            printf("vkBeginCommandBuffer returned error.\n");
            return;
        }

        VkImageMemoryBarrier imageMemoryBarrier;
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageMemoryBarrier.pNext = NULL;
        imageMemoryBarrier.image = engine->swapChainImages[currentBuffer];
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
        imageMemoryBarrier.subresourceRange.levelCount = 1;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
        imageMemoryBarrier.subresourceRange.layerCount = 1;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkPipelineStageFlags destStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        vkCmdPipelineBarrier(engine->renderCommandBuffer[i], srcStageFlags, destStageFlags, 0,
                             0, NULL, 0, NULL, 1, &imageMemoryBarrier);

        vkCmdBeginRenderPass(engine->renderCommandBuffer[i], &renderPassBeginInfo,
                             VK_SUBPASS_CONTENTS_INLINE);
        if (i == 1) {
            vkCmdBindPipeline(engine->renderCommandBuffer[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                              engine->pipeline[i]);
            vkCmdBindDescriptorSets(engine->renderCommandBuffer[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    engine->pipelineLayout, 0, 1,
                                    engine->descriptorSets, 0, NULL);

//    vkCmdSetViewport(engine->renderCommandBuffer, 0, 1, &viewport);
//    vkCmdSetScissor(engine->renderCommandBuffer, 0, 1, &scissor);

            VkDeviceSize offsets[1] = {0};
            vkCmdBindVertexBuffers(engine->renderCommandBuffer[i], 0, 1, &engine->vertexBuffer,
                                   offsets);
            vkCmdDraw(engine->renderCommandBuffer[i], 12 * 3, 1, 0, 0);
        }

        VkClearAttachment clear;
        clear.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        clear.clearValue.color.float32[0]=i;
        clear.clearValue.color.float32[1]=0.5;
        clear.clearValue.color.float32[2]=0;
        clear.clearValue.color.float32[3]=0;
        clear.colorAttachment=0;
        VkClearRect clearRect;
        clearRect.baseArrayLayer=0;
        clearRect.layerCount=1;
        clearRect.rect.extent.width=engine->width/4*2;
        clearRect.rect.extent.height=engine->height/4*2;
        clearRect.rect.offset.x=engine->width/4 - (i * 100);
        clearRect.rect.offset.y=engine->height/4;
//        vkCmdClearAttachments(engine->renderCommandBuffer[i], 1, &clear, 1, &clearRect);

        vkCmdEndRenderPass(engine->renderCommandBuffer[i]);

        VkImageMemoryBarrier prePresentBarrier;
        prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        prePresentBarrier.pNext = NULL;
        prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        prePresentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        prePresentBarrier.subresourceRange.baseMipLevel = 0;
        prePresentBarrier.subresourceRange.levelCount = 1;
        prePresentBarrier.subresourceRange.baseArrayLayer = 0;
        prePresentBarrier.subresourceRange.layerCount = 1;
        prePresentBarrier.image = engine->swapChainImages[currentBuffer];
        vkCmdPipelineBarrier(engine->renderCommandBuffer[i], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0,
                             NULL, 1, &prePresentBarrier);

        res = vkEndCommandBuffer(engine->renderCommandBuffer[i]);
        if (res != VK_SUCCESS) {
            LOGE ("vkEndCommandBuffer returned error %d.\n", res);
            return;
        }
    }

    VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkSubmitInfo submitInfo[2];
    submitInfo[0].pNext = NULL;
    submitInfo[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo[0].waitSemaphoreCount = 1; //Only the first command buffer should wait on a semaphore,
    submitInfo[0].pWaitSemaphores = &engine->presentCompleteSemaphore;
    submitInfo[0].pWaitDstStageMask = &pipe_stage_flags;
    submitInfo[0].commandBufferCount = 2;
    submitInfo[0].pCommandBuffers = engine->renderCommandBuffer;
    submitInfo[0].signalSemaphoreCount = 0;
    submitInfo[0].pSignalSemaphores = NULL;
    submitInfo[1].pNext = NULL;
    submitInfo[1].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo[1].waitSemaphoreCount = 0; //Only the first command buffer should wait on a semaphore,
    submitInfo[1].pWaitSemaphores = NULL;
    submitInfo[1].pWaitDstStageMask = &pipe_stage_flags;
    submitInfo[1].commandBufferCount = 1;
    submitInfo[1].pCommandBuffers = &engine->renderCommandBuffer[1];
    submitInfo[1].signalSemaphoreCount = 0;
    submitInfo[1].pSignalSemaphores = NULL;

    res = vkQueueSubmit(engine->queue, 1, submitInfo, VK_NULL_HANDLE);
    if (res != VK_SUCCESS) {
        LOGE ("vkQueueSubmit returned error %d.\n", res);
        return;
    }
//
//    res = vkQueueSubmit(engine->queue, 1, &submitInfo[1], VK_NULL_HANDLE);
//    if (res != VK_SUCCESS) {
//        LOGE ("vkQueueSubmit returned error %d.\n", res);
//        return;
//    }
//
//    res = vkQueueSubmit(engine->queue, 1, &submitInfo[1], VK_NULL_HANDLE);
//    if (res != VK_SUCCESS) {
//        LOGE ("vkQueueSubmit returned error %d.\n", res);
//        return;
//    }
//
//    res = vkQueueSubmit(engine->queue, 1, &submitInfo[1], VK_NULL_HANDLE);
//    if (res != VK_SUCCESS) {
//        LOGE ("vkQueueSubmit returned error %d.\n", res);
//        return;
//    }

    LOGI ("Waiting.\n");

    res = vkQueueWaitIdle(engine->queue);
    if (res != VK_SUCCESS) {
        LOGE ("vkQueueSubmit returned error %d.\n", res);
        return;
    }

    LOGI ("Presentng.\n");

    VkPresentInfoKHR presentInfo;
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = NULL;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &engine->swapchain;
    presentInfo.pImageIndices = &currentBuffer;
    presentInfo.pWaitSemaphores = NULL;
    presentInfo.waitSemaphoreCount = 0;
    presentInfo.pResults = NULL;
    res = vkQueuePresentKHR(engine->queue, &presentInfo);
    if (res != VK_SUCCESS) {
        LOGE ("vkQueuePresentKHR returned error %d.\n", res);
        return;
    }

    LOGI ("Finished frame %d.\n", engine->frame);
    engine->frame++;
}

/**
 * Tear down the EGL context currently associated with the display.
 */
static void engine_term_display(struct engine* engine) {
    LOGI("engine_term_display");
//    vkDestroyImageView(engine->vkDevice, engine->depthView, NULL);
//    vkDestroyImage(engine->vkDevice, engine->depthImage, NULL);
//    vkFreeMemory(engine->vkDevice, engine->depthMemory, NULL);
//    if (engine->display != EGL_NO_DISPLAY) {
//        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
//        if (engine->context != EGL_NO_CONTEXT) {
//            eglDestroyContext(engine->display, engine->context);
//        }
//        if (engine->surface != EGL_NO_SURFACE) {
//            eglDestroySurface(engine->display, engine->surface);
//        }
//        eglTerminate(engine->display);
//    }
//    engine->animating = 0;
//    engine->display = EGL_NO_DISPLAY;
//    engine->context = EGL_NO_CONTEXT;
//    engine->surface = EGL_NO_SURFACE;
}

/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
    struct engine* engine = (struct engine*)app->userData;
//    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
    LOGI("engine_handle_input");
        engine->animating = 1;
        engine->state.x = AMotionEvent_getX(event, 0);
        engine->state.y = AMotionEvent_getY(event, 0);
        return 1;
//    }
//    return 0;
}

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            // The system has asked us to save our current state.  Do so.
            engine->app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)engine->app->savedState) = engine->state;
            engine->app->savedStateSize = sizeof(struct saved_state);
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (engine->app->window != NULL) {
                engine_init_display(engine);
                engine_draw_frame(engine);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            engine_term_display(engine);
            break;
        case APP_CMD_GAINED_FOCUS:
            // When our app gains focus, we start monitoring the accelerometer.
            if (engine->accelerometerSensor != NULL) {
                ASensorEventQueue_enableSensor(engine->sensorEventQueue,
                                               engine->accelerometerSensor);
                // We'd like to get 60 events per second (in us).
                ASensorEventQueue_setEventRate(engine->sensorEventQueue,
                                               engine->accelerometerSensor,
                                               (1000L/60)*1000);
            }
            break;
        case APP_CMD_LOST_FOCUS:
            // When our app loses focus, we stop monitoring the accelerometer.
            // This is to avoid consuming battery while not being used.
            if (engine->accelerometerSensor != NULL) {
                ASensorEventQueue_disableSensor(engine->sensorEventQueue,
                                                engine->accelerometerSensor);
            }
            // Also stop animating.
            LOGI("APP_CMD_LOST_FOCUS");
            engine->animating = 0;
            engine_draw_frame(engine);
            break;
    }
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {
    struct engine engine;

    // Make sure glue isn't stripped.
    app_dummy();

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app = state;
    engine.animating=1;
    engine.vulkanSetupOK=false;

    // Prepare to monitor accelerometer
    engine.sensorManager = ASensorManager_getInstance();
    engine.accelerometerSensor = ASensorManager_getDefaultSensor(
                                        engine.sensorManager,
                                        ASENSOR_TYPE_ACCELEROMETER);
    engine.sensorEventQueue = ASensorManager_createEventQueue(
                                    engine.sensorManager,
                                    state->looper, LOOPER_ID_USER,
                                    NULL, NULL);

    if (state->savedState != NULL) {
        // We are starting with a previous saved state; restore from it.
        engine.state = *(struct saved_state*)state->savedState;
    }

    // loop waiting for stuff to do.

    while (1) {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
//        LOGI("Polling %d", engine.animating);
        while ((ident=ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events,
                                      (void**)&source)) >= 0) {
//            LOGI("Poll returned");

            // Process this event.
            if (source != NULL) {
                source->process(state, source);
            }

            // If a sensor has data, process it now.
//            if (ident == LOOPER_ID_USER) {
//                if (engine.accelerometerSensor != NULL) {
//                    ASensorEvent event;
//                    while (ASensorEventQueue_getEvents(engine.sensorEventQueue,
//                                                       &event, 1) > 0) {
//                        LOGI("accelerometer: x=%f y=%f z=%f",
//                             event.acceleration.x, event.acceleration.y,
//                             event.acceleration.z);
//                    }
//                }
//            }

            // Check if we are exiting.
            if (state->destroyRequested != 0) {
                engine_term_display(&engine);
                return;
            }
        }
//        LOGI("Cont");

        if (engine.animating) {
            // Done with events; draw next animation frame.
            engine.state.angle += .01f;
            if (engine.state.angle > 1) {
                engine.state.angle = 0;
            }

            // Drawing is throttled to the screen update rate, so there
            // is no need to do timing here.
            engine_draw_frame(&engine);
        }
    }
}
//END_INCLUDE(all)
