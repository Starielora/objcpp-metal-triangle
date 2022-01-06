#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <imgui.h>
#include <imgui_impl_metal.h>

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/QuartzCore.h>

namespace
{
    void quit(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }
}

auto* createWindow()
{
    if (glfwInit() != GLFW_TRUE)
    {
        std::cerr << "Failed to init glfw.\n";
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    
    const auto window = glfwCreateWindow(800, 600, "objcpp-metal-triangle", nullptr, nullptr);
    
    if (window == nullptr)
    {
        glfwTerminate();
        std::cerr << "Failed to create window.\n";
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, quit);

    return window;
}

auto readFile(std::filesystem::path p)
{
    auto file = std::ifstream(p);
    assert(file.is_open());
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

struct Vec2
{
    float x, y;
};

struct Vec3
{
    float x, y, z;
};

struct Vertex
{
    Vec2 pos;
    Vec3 color;
};

int main()
{
    const auto shaderSrc = readFile("/Users/patrykedyko/dev/objcpp-metal-triangle/assets/shaders.metal");
    
    const id<MTLDevice> gpu = MTLCreateSystemDefaultDevice();
    const id<MTLCommandQueue> queue = [gpu newCommandQueue];
    CAMetalLayer *swapchain = [CAMetalLayer layer];
    swapchain.device = gpu;
    swapchain.opaque = YES;
    
    auto* const window = createWindow();
    
    NSWindow* nswindow = glfwGetCocoaWindow(window);
    nswindow.contentView.layer = swapchain;
//    nswindow.contentView.wantsLayer = YES;

    MTLClearColor color = MTLClearColorMake(75.f/255.f, 100.f/255.f, 200.f/255.f, 1);

    NSError* errors;
    MTLCompileOptions* options = [MTLCompileOptions new];
    id<MTLLibrary> library = [gpu newLibraryWithSource:[NSString stringWithCString:shaderSrc.c_str()] options:options error:&errors];
    assert(!errors);
    id<MTLFunction> vertexShader = [library newFunctionWithName:@"vertex_main"];
    id<MTLFunction> fragmentShader = [library newFunctionWithName:@"fragment_main"];
    MTLRenderPipelineDescriptor* pipelineDescriptor = [MTLRenderPipelineDescriptor new];
    pipelineDescriptor.vertexFunction = vertexShader;
    pipelineDescriptor.fragmentFunction = fragmentShader;
    pipelineDescriptor.vertexDescriptor.attributes[0].format = MTLVertexFormatFloat2;
    pipelineDescriptor.vertexDescriptor.attributes[0].offset = 0;
    pipelineDescriptor.vertexDescriptor.attributes[0].bufferIndex = 0;
    pipelineDescriptor.vertexDescriptor.attributes[1].format = MTLVertexFormatFloat3;
    pipelineDescriptor.vertexDescriptor.attributes[1].offset = 2 * sizeof(float);
    pipelineDescriptor.vertexDescriptor.attributes[1].bufferIndex = 0;
    pipelineDescriptor.vertexDescriptor.layouts[0].stride = 5 * sizeof(float);
    pipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

    id<MTLRenderPipelineState> pipelineState = [gpu newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&errors];
    
    assert(pipelineState && !errors);
    
    Vertex top = {{ 0.0,  0.5}, {1.0, 0.0, 0.0}};
    Vertex bottomLeft = {{-0.5, -0.5}, {0.0, 1.0, 0.0}};
    Vertex bottomRight = {{ 0.5, -0.5}, {0.0, 0.0, 1.0}};
    Vertex vertices[] = { top, bottomLeft, bottomRight };

    float time = 0.0;
    
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        @autoreleasepool {
            id<CAMetalDrawable> surface = [swapchain nextDrawable];

            MTLRenderPassDescriptor *pass = [MTLRenderPassDescriptor renderPassDescriptor];
            pass.colorAttachments[0].clearColor = color;
            pass.colorAttachments[0].loadAction  = MTLLoadActionClear;
            pass.colorAttachments[0].storeAction = MTLStoreActionStore;
            pass.colorAttachments[0].texture = surface.texture;

            id<MTLCommandBuffer> buffer = [queue commandBuffer];
            id<MTLRenderCommandEncoder> encoder = [buffer renderCommandEncoderWithDescriptor:pass];
            [encoder setVertexBytes: vertices length: 3 * sizeof(Vertex) atIndex:0];
            [encoder setRenderPipelineState:pipelineState];
            [encoder drawPrimitives: MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
            [encoder endEncoding];
            [buffer presentDrawable:surface];
            [buffer commit];
        }
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
