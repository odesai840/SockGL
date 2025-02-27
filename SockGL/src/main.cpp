#include <iostream>
#include <set>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SOIL2.h>

#include "shader.h"
#include "camera.h"
#include "input.h"
#include "model.h"

// debugging
std::set<unsigned int> debugLogs;
bool debugNormals = false;
bool debugSpec = false;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;

// input
Input input;
bool firstMouse = true;
bool captureMouse = true;
bool freezeInput = false;

// frame timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

// viewport
GLuint VIEW_WIDTH = 800;
GLuint VIEW_HEIGHT = 600;
GLuint viewportFBO; // viewport frame buffer object
GLuint viewportRBO; // viewport render buffer object
GLuint texture_id; // the texture id to create a texture

// lighting
glm::vec3 dirLightDir(-0.2f, -1.0f, -0.3f);
float lightDistance = 250.0f;

// shadow mapping
const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;
unsigned int depthMapFBO;
unsigned int depthMap;
float shadowBias = 0.0025f;
float near_plane = 0.1f;
float far_plane = 500.0f;
float orthoSize = 250.0f;

GLenum glCheckError_(const char* file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
        case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
        case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
        case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
        case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
        case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void APIENTRY glDebugOutput(GLenum source,
    GLenum type,
    unsigned int id,
    GLenum severity,
    GLsizei length,
    const char* message,
    const void* userParam)
{
    // ignore these non-significant error codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) {
        return;
    }
    
    // Check if the message ID has already been logged
    if (debugLogs.find(id) != debugLogs.end())
        return; // Skip if already logged

    // Add the current message ID to the set
    debugLogs.insert(id);

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source) {
    case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type) {
    case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processGamepadLookInput() {
    if (glfwJoystickPresent(GLFW_JOYSTICK_1)) {
        int axesCount;
        const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);

        if (axesCount >= 4) { // Right stick usually corresponds to axes 2 (X) and 3 (Y)
            float rightStickX = axes[2]; // Right stick X-axis
            float rightStickY = axes[3]; // Right stick Y-axis

            // Threshold to prevent drift
            const float deadZone = 0.2f;
            if (std::abs(rightStickX) > deadZone || std::abs(rightStickY) > deadZone) {
                float sensitivity = 10.0f; // Adjust this value for desired responsiveness

                float xoffset = rightStickX * sensitivity;
                float yoffset = -rightStickY * sensitivity; // Invert Y-axis to match mouse behavior

                if (!freezeInput) {
                    camera.ProcessMouseMovement(xoffset, yoffset);
                }
            }
        }
    }
}

void processGamepadZoomInput(int buttonCount, const unsigned char* buttons) {
    if (buttonCount > 0 && buttons) {
        if (buttons[0] == GLFW_PRESS) {
            camera.ProcessMouseScroll(0.5f);
        }
        if (buttonCount > 1 && buttons[1] == GLFW_PRESS) {
            camera.ProcessMouseScroll(-0.5f);
        }
    }
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window) {
    if (!freezeInput) {
        // keyboard controls
        if (input.GetKeyHeld(GLFW_KEY_W)) {
            camera.ProcessKeyboard(FORWARD, deltaTime);
        }
        if (input.GetKeyHeld(GLFW_KEY_A)) {
            camera.ProcessKeyboard(LEFT, deltaTime);
        }
        if (input.GetKeyHeld(GLFW_KEY_S)) {
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        }
        if (input.GetKeyHeld(GLFW_KEY_D)) {
            camera.ProcessKeyboard(RIGHT, deltaTime);
        }

        // gamepad controls
        if (glfwJoystickPresent(GLFW_JOYSTICK_1)) {
            int axesCount;
            const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);
            int buttonCount;
            const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonCount);

            if (axesCount >= 2) {
                if (axes[1] < -0.2f) {
                    camera.ProcessKeyboard(FORWARD, deltaTime * axes[1] * -1.0f);
                }
                if (axes[1] > 0.2f) {
                    camera.ProcessKeyboard(BACKWARD, deltaTime * axes[1]);
                }
                if (axes[0] < -0.2f) {
                    camera.ProcessKeyboard(LEFT, deltaTime * axes[0] * -1.0f);
                }
                if (axes[0] > 0.2f) {
                    camera.ProcessKeyboard(RIGHT, deltaTime * axes[0]);
                }
            }

            processGamepadLookInput();
            processGamepadZoomInput(buttonCount, buttons);
        }
    }

    if (input.GetKeyPressed(GLFW_KEY_ESCAPE)) {
        freezeInput = !freezeInput;
        if (captureMouse) {
            captureMouse = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else {
            captureMouse = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}

// glfw: whenever a key on the keyboard is pressed, this callback is called
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // updates state of the pressed key in the input class
    input.UpdateKeyState(key, action);
}

// glfw: whenever the mouse moves, this callback is called
void cursor_pos_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (!freezeInput) {
        camera.ProcessMouseMovement(xoffset, yoffset);
    }
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (!freezeInput) {
        camera.ProcessMouseScroll(static_cast<float>(yoffset));
    }
}

// create framebuffer and renderbuffer
void create_framebuffer(GLuint* FBO, GLuint* RBO)
{
    glGenFramebuffers(1, FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, *FBO);

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, VIEW_WIDTH, VIEW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);

    glGenRenderbuffers(1, RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, *RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, VIEW_WIDTH, VIEW_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, *RBO);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// bind framebuffer
void bind_framebuffer(GLuint* FBO)
{
    glBindFramebuffer(GL_FRAMEBUFFER, *FBO);
}

// unbind framebuffer
void unbind_framebuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// rescale the buffer to allow resizing of window
void rescale_framebuffer(GLuint* FBO, GLuint* RBO, float width, float height)
{
    glBindFramebuffer(GL_FRAMEBUFFER, *FBO);

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, *RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, *RBO);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void setupShadowMap(unsigned int* shadowFBO, unsigned int* depthMap, unsigned int width, unsigned int height)
{
    // Generate framebuffer
    glGenFramebuffers(1, shadowFBO);
    
    // Create depth texture
    glGenTextures(1, depthMap);
    glBindTexture(GL_TEXTURE_2D, *depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    
    // Attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, *shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main() {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef _DEBUG
    glfwWindowHint(GLFW_CONTEXT_DEBUG, true);
#endif

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);


#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "SockGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwMaximizeWindow(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    // initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // ImGui io and config flags
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // set ImGui style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // intialize ImGui backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    // set window icon
    GLFWimage images[1];
    images[0].pixels = SOIL_load_image("resources/sockenginelogo.png", &images[0].width, &images[0].height, 0, 4);
    glfwSetWindowIcon(window, 1, images);
    SOIL_free_image_data(images[0].pixels);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // enable OpenGL debug context if context allows for debug context
    int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // makes sure errors are displayed synchronously
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    // build and compile our shader program
    Shader modelShader("shaders/model.vert", "shaders/model.frag");
    Shader shadowMapShader("shaders/shadowmap.vert", "shaders/shadowmap.frag");
    Shader skyboxShader("shaders/skybox.vert","shaders/skybox.frag");
    /*
    Shader lightShader("shaders/cube.vert", "shaders/cube.frag");

    float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    glm::vec3 pointLightPosition = glm::vec3(0.7f, 0.2f, 2.0f);

    // configure the light's VAO and VBO
    unsigned int lightCubeVBO, lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glGenBuffers(1, &lightCubeVBO);

    glBindBuffer(GL_ARRAY_BUFFER, lightCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(lightCubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    */

    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    std::vector<std::string> skyboxFaces {
        "resources/textures/skybox/right.bmp",
        "resources/textures/skybox/left.bmp",
        "resources/textures/skybox/top.bmp",
        "resources/textures/skybox/bottom.bmp",
        "resources/textures/skybox/front.bmp",
        "resources/textures/skybox/back.bmp"
    };
    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    unsigned int cubemapTexture = Model::loadCubemap(skyboxFaces);

    Model sponza("resources/models/sponza/sponza.obj");

    // uncomment this line to draw in wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    create_framebuffer(&viewportFBO, &viewportRBO);
    setupShadowMap(&depthMapFBO, &depthMap, SHADOW_WIDTH, SHADOW_HEIGHT);

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // frame rate and render time calculations
        float fps = 1.0f / deltaTime;
        float renderTime = deltaTime * 1000.0f;

        // input
        processInput(window);

        // start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // setup ImGui dock space
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus;
        ImGui::Begin("Main Dockspace", nullptr, window_flags);
        ImGuiID dockspace_id = ImGui::GetID("Main Dockspace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
        ImGui::End();
        
        // show stats window
        ImGui::Begin("Stats");
        ImGui::Text("FPS: %.1f\nRender time: %.3f ms", fps, renderTime);
        ImGui::End();
        
        // show debugging window
        ImGui::Begin("Debugging");
        if (ImGui::Checkbox("Debug Normals", &debugNormals)) {
            if (debugNormals) {
                debugSpec = false;
            }
        }
        if (ImGui::Checkbox("Debug Specular", &debugSpec)) {
            if (debugSpec) {
                debugNormals = false;
            }
        }
        ImGui::Separator();
        ImGui::Text("Shadow Settings");
        ImGui::SliderFloat("Shadow Bias", &shadowBias, 0.0f, 0.05f, "%.5f");
        ImGui::SliderFloat("Near Plane", &near_plane, 0.1f, 1.0f, "%.5f");
        ImGui::SliderFloat("Far Plane", &far_plane, 0.1f, 500.0f, "%.5f");
        ImGui::SliderFloat("Ortho Size", &orthoSize, 1.0f, 250.0f, "%.5f");
        ImGui::SliderFloat("Light Distance", &lightDistance, 1.0f, 250.0f, "%.5f");
        ImGui::End();

        // show viewport window
        ImGui::Begin("Viewport");
        // ImGui window size
        const float window_width = ImGui::GetContentRegionAvail().x;
        const float window_height = ImGui::GetContentRegionAvail().y;
        // rescale framebuffer
        rescale_framebuffer(&viewportFBO, &viewportRBO, window_width, window_height);
        glViewport(0, 0, window_width, window_height);
        // get screen position of window
        ImVec2 pos = ImGui::GetCursorScreenPos();
        // add created texture as image to ImGui
        ImGui::GetWindowDrawList()->AddImage(
            (void*)texture_id,
            ImVec2(pos.x, pos.y),
            ImVec2(pos.x + window_width, pos.y + window_height),
            ImVec2(0, 1),
            ImVec2(1, 0)
        );
        ImGui::End();
        
        // first render pass: render depth map from light's perspective
        glm::mat4 lightProjection, lightView, lightSpaceMatrix;

        // calculate light space matrix for directional light
        lightProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, near_plane, far_plane);
        lightView = glm::lookAt(-dirLightDir * lightDistance,
                               glm::vec3(0.0f),
                               glm::vec3(0.0f, 1.0f, 0.0f));
        lightSpaceMatrix = lightProjection * lightView;
        
        // render to depth map
        shadowMapShader.use();
        shadowMapShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        // configure viewport to shadow map dimensions
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        
        // adjust face culling
        glDisable(GL_CULL_FACE);
        
        // Render the scene from the light's POV
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        shadowMapShader.setMat4("model", model);
        sponza.Draw(shadowMapShader);
        
        // Reset face culling
        glEnable(GL_CULL_FACE);
        
        // Reset framebuffer and viewport
        glBindFramebuffer(GL_FRAMEBUFFER, viewportFBO);
        glViewport(0, 0, window_width, window_height);

        // second render pass: render scene normally using the generated depth map
        // render
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // enable shader before setting uniforms
        modelShader.use();
        
        // set lighting uniforms as before
        modelShader.setBool("debugNormals", debugNormals);
        modelShader.setBool("debugSpec", debugSpec);
        modelShader.setVec3("viewPos", camera.Position);
        modelShader.setFloat("material.shininess", 32.0f);
        
        // directional light
        modelShader.setVec3("dirLight.direction", dirLightDir);
        modelShader.setVec3("dirLight.ambient", 0.1f, 0.1f, 0.1f);
        modelShader.setVec3("dirLight.diffuse", 1.0f, 1.0f, 1.0f);
        modelShader.setVec3("dirLight.specular", 0.3f, 0.3f, 0.3f);
        
        // shadow mapping uniforms
        modelShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        modelShader.setFloat("shadowBias", shadowBias);
        
        // bind shadow map
        glActiveTexture(GL_TEXTURE0 + 5);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        modelShader.setInt("shadowMap", 5);
        
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)window_width / (float)window_height, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);
        
        // render the model
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        modelShader.setMat4("model", model);
        sponza.Draw(modelShader);

        if (!debugNormals && !debugSpec) {
            // disable culling
            glDisable(GL_CULL_FACE);

            /* draw the light object(s)
            lightShader.use();
            lightShader.setMat4("projection", projection);
            lightShader.setMat4("view", view);
            glBindVertexArray(lightCubeVAO);
            model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPosition);
            model = glm::scale(model, glm::vec3(0.2f));
            lightShader.setMat4("model", model);
            // comment the line below to make light object invisible
            glDrawArrays(GL_TRIANGLES, 0, 36);
            */
            
            // draw skybox as last
            glDepthFunc(GL_LEQUAL); // change depth function so depth test passes when values are equal to depth buffer's content
            skyboxShader.use();
            view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
            skyboxShader.setMat4("view", view);
            skyboxShader.setMat4("projection", projection);
            // skybox cube
            glBindVertexArray(skyboxVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
            glDepthFunc(GL_LESS); // set depth function back to default

            // re-enable culling
            glEnable(GL_CULL_FACE);
        }

        // unbind framebuffer
        unbind_framebuffer();

        // render ImGui windows
        ImGui::Render();

        // end ImGui frame
        ImGui::EndFrame();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // deallocate resources
    //glDeleteVertexArrays(1, &lightCubeVAO);
    //glDeleteBuffers(1, &lightCubeVBO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);

    // shutdown ImGui
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
    
    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}
