#include "interactive/InteractiveSession.hpp"
#include "Renderer.hpp"
// dear imgui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>

// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>            // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>            // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>          // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h>  // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h>// Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

using namespace std;

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void InteractiveSession::initRenderTexture(int width, int height) {

    // Make framebuffer that Sketch renders to

    glGenFramebuffers(1, &sketchFramebuffer);

    // Bind Framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, sketchFramebuffer);

    // The texture we're going to render to
    glGenTextures(1, &sketchTexture);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, sketchTexture);

    // Give an empty image to OpenGL ( the last "0" )
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);

    // Poor filtering. Needed !
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // The depth buffer
    glGenRenderbuffers(1, &depthrenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

    // Set "renderedTexture" as our colour attachement #0
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sketchTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        cout << "Error creating sketch framebuffer." << endl;
    }

    // change drawing target back to default framebuffer so that imgui can do its thing
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void InteractiveSession::updateFrame() {
    if (!frameNeedsUpdate) return;
    if (rendererNeedsUpdate) {
        mtr = make_shared <MultiThreadRenderer> (camera, scene, tracer, image, options.spp, options.nt);
    }
    thread t{ [this]() {

        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        mtr->render();
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        cout << endl << "Render complete." << endl;
        std::chrono::milliseconds rt_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        cout << utils::format_duration(rt_ms) << endl;

        }
    };
    t.detach();
    frameNeedsUpdate = rendererNeedsUpdate = false;
}

float* paneAsFloats = new float[3 * 64 * 64];

void InteractiveSession::updateTextureIfNecessary() {
    
    for (auto& p : mtr->getPanes()) {
        if (p.updated) {
            cout << "u";
            
            for (int row = p.y0; row < p.y0 + p.h; row++) {
                for (int col = p.x0; col < p.x0 + p.w; col++) {
                    int index = ((row - p.y0) * p.w + (col - p.x0)) * 3;
                    Color& c = image(col, row);
                    paneAsFloats[index] = c.r;
                    paneAsFloats[index + 1] = c.g;
                    paneAsFloats[index + 2] = c.b;

                }
            }
            glTexSubImage2D(GL_TEXTURE_2D, 0, p.x0, p.y0, p.w, p.h, GL_RGB, GL_FLOAT, paneAsFloats);

            p.updated = false;
        }
    }
}

void InteractiveSession::go() {
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return;

    // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
    bool err = false;
    glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
    bool err = false;
    glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return;
    }

    initRenderTexture(options.imageWidth, options.imageHeight);
    frameNeedsUpdate = true;
    rendererNeedsUpdate = true;    

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // Image window
        ImGui::SetNextWindowPos(ImVec2(450, 50), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(options.imageWidth, options.imageHeight), ImGuiCond_Once);
        ImGui::SetNextWindowBgAlpha(0.0f);
        // Render the sketch
        glBindFramebuffer(GL_FRAMEBUFFER, sketchFramebuffer);

        glViewport(0, 0, options.imageWidth, options.imageHeight);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
        ImGui::Begin("Sketch", &show_another_window, ImGuiWindowFlags_NoScrollbar);
        int ulx = ImGui::GetWindowContentRegionMin().x + ImGui::GetWindowPos().x;
        int uly = ImGui::GetWindowContentRegionMin().y + ImGui::GetWindowPos().y;
        int cw = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
        int ch = ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y;
        float normX = 2.0f * (((float)ImGui::GetMousePos().x - (float)ulx) / (float)cw - 0.5f);
        float normY = 2.0f * (((float)ImGui::GetMousePos().y - (float)uly) / (float)ch - 0.5f);

        //s.setMousePos(normX, normY);
        //s.setClickState(io.MouseDown[0], io.MouseDown[1], io.MouseDown[2]);
        //if (ImGui::IsMouseDragging(0)) s.onMouseDrag(0);
        //if (ImGui::GetIO().MouseWheel != 0.0) s.onMouseWheel(ImGui::GetIO().MouseWheel);
        //s.draw(ImGui::GetTime());
        updateFrame();
        updateTextureIfNecessary();

        /*
        if (recording) {
            stringstream ss;
            ss << "Recording...\t" << curFrame << " frames.";
            ImGui::Text(ss.str().c_str());
        }
        */
        ImVec2 vMin = ImGui::GetWindowContentRegionMin();
        ImVec2 vMax; // = ImGui::GetWindowContentRegionMax();

        vMin.x += ImGui::GetWindowPos().x;
        vMin.y += ImGui::GetWindowPos().y;
        //vMax.x += ImGui::GetWindowPos().x;
        //vMax.y += ImGui::GetWindowPos().y;

        vMax.x = vMin.x + cw;
        vMax.y = vMin.y + ch;

        //ImGui::GetForegroundDrawList()->AddRectFilled( vMin, vMax, IM_COL32( 255, 255, 0, 255 ) );
        ImGui::ImageButton((void*)(uintptr_t)sketchTexture, ImVec2(cw, ch), ImVec2(0.f, 0.f), ImVec2(1.f, 1.f));
        /*
        if (recording) {
            glBindFramebuffer(GL_FRAMEBUFFER, sketchFramebuffer);
            //glReadPixels(0, 0, sketchWidth, sketchHeight, GL_RGBA, GL_UNSIGNED_BYTE, imageData[curFrame++].data());
            glGetTextureImage(sketchTexture, 0, GL_RGBA, GL_UNSIGNED_BYTE, frameSize, imageData[curFrame++].data());
            //curFrame++;
            if (glCheckError())
                exit(-1);

        }
*/
        ImGui::End();
        ImGui::PopStyleVar();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    delete[] paneAsFloats;

    return;
}