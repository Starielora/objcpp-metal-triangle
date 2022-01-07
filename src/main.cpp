#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

void quit(GLFWwindow *window, int key, int scancode, int action, int mods);
GLFWwindow* createWindow(CAMetalLayer* metalLayer);
CAMetalLayer* createMetalLayer(id<MTLDevice> gpu);
std::string readFile(std::filesystem::path p);
id<MTLRenderPipelineState> createPipelineState(id<MTLDevice> gpu);

struct Vec2 { float x, y; };
struct Vec3 { float x, y, z; };
struct Vertex { Vec2 pos; Vec3 color; };

int main()
{
    const id<MTLDevice> gpu = MTLCreateSystemDefaultDevice();
    const id<MTLCommandQueue> queue = [gpu newCommandQueue];
    auto* const metalLayer = createMetalLayer(gpu);
    auto* const window = createWindow(metalLayer);
    const auto pipelineState = createPipelineState(gpu);

    // triangle
    Vertex top = {{ 0.0,  0.5}, {1.0, 0.0, 0.0}};
    Vertex bottomLeft = {{-0.5, -0.5}, {0.0, 1.0, 0.0}};
    Vertex bottomRight = {{ 0.5, -0.5}, {0.0, 0.0, 1.0}};
    Vertex vertices[] = { top, bottomLeft, bottomRight };

    float time = 0.0;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        time = time == 1.0 ? 0 : time + 0.01;

        top.color.x = 0.5 * (sin(time) + 1);
        bottomLeft.color.y = 0.5 * (sin(time + 3.14/2) + 1);
        bottomRight.color.z = 0.5 * (sin(time + 3.14) + 1);

        Vertex vertices[] = { top, bottomLeft, bottomRight };

        @autoreleasepool {
            id<CAMetalDrawable> surface = [metalLayer nextDrawable];

            MTLRenderPassDescriptor *pass = [MTLRenderPassDescriptor renderPassDescriptor];
//            pass.colorAttachments[0].clearColor = color;
            pass.colorAttachments[0].loadAction  = MTLLoadActionClear;
            pass.colorAttachments[0].storeAction = MTLStoreActionStore;
            pass.colorAttachments[0].texture = surface.texture;

            id<MTLCommandBuffer> buffer = [queue commandBuffer];
            id<MTLRenderCommandEncoder> encoder = [buffer renderCommandEncoderWithDescriptor:pass];
            [encoder setVertexBytes: vertices length: sizeof(vertices) atIndex:0];
            [encoder setRenderPipelineState:pipelineState];
            [encoder drawPrimitives: MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount: sizeof(vertices) / sizeof(Vertex)];
            [encoder endEncoding];
            [buffer presentDrawable:surface];
            [buffer commit];
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}

void quit(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

GLFWwindow* createWindow(CAMetalLayer* metalLayer)
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

    NSWindow* nswindow = glfwGetCocoaWindow(window);
    // Could assign MTKView here, but GLFW implements its own NSView which handles keyboard and mouse events
    nswindow.contentView.layer = metalLayer;
    nswindow.contentView.wantsLayer = YES;

    return window;
}

CAMetalLayer* createMetalLayer(id<MTLDevice> gpu)
{
    CAMetalLayer *swapchain = [CAMetalLayer layer];
    swapchain.device = gpu;
    swapchain.opaque = YES;

    return swapchain;
}

std::string readFile(std::filesystem::path p)
{
    auto file = std::ifstream(p);
    assert(file.is_open());
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

id<MTLRenderPipelineState> createPipelineState(id<MTLDevice> gpu)
{
    const auto shaderSrc = readFile("../../assets/shaders.metal"); // back to root

    NSError* errors;
    id<MTLLibrary> library = [gpu newLibraryWithSource:[NSString stringWithCString:shaderSrc.c_str() encoding:NSASCIIStringEncoding] options:nullptr error:&errors];
    assert(!errors);

    id<MTLFunction> vertexShader = [library newFunctionWithName:@"vertex_main"];
    id<MTLFunction> fragmentShader = [library newFunctionWithName:@"fragment_main"];
    MTLRenderPipelineDescriptor* pipelineDescriptor = [MTLRenderPipelineDescriptor new];

    // set shaders
    pipelineDescriptor.vertexFunction = vertexShader;
    pipelineDescriptor.fragmentFunction = fragmentShader;

    // vertex position attribute
    pipelineDescriptor.vertexDescriptor.attributes[0].format = MTLVertexFormatFloat2;
    pipelineDescriptor.vertexDescriptor.attributes[0].offset = offsetof(Vertex, pos);
    pipelineDescriptor.vertexDescriptor.attributes[0].bufferIndex = 0;

    // vertex color attribute
    pipelineDescriptor.vertexDescriptor.attributes[1].format = MTLVertexFormatFloat3;
    pipelineDescriptor.vertexDescriptor.attributes[1].offset = offsetof(Vertex, color);
    pipelineDescriptor.vertexDescriptor.attributes[1].bufferIndex = 0;

    // data stride
    pipelineDescriptor.vertexDescriptor.layouts[0].stride = sizeof(Vertex);

    // texture format
    pipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

    id<MTLRenderPipelineState> pipelineState = [gpu newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&errors];
    
    assert(pipelineState && !errors);

    return pipelineState;
}
