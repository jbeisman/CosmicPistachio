
// These must be included in exactly one source file
#define SDL_MAIN_USE_CALLBACKS
#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif

#include <fstream>
#include <iostream>
#include <memory>

#include "renderer.hh"
#include "camera.hh"
#include "system.hh"

#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

class AppState {
public:
  AppState();
  ~AppState() = default;
  SDL_Window *window_ptr;
  SDL_GLContext context_ptr;
  ImVec4 clear_color{ImVec4(0.45f, 0.55f, 0.60f, 1.00f)};
  bool app_finished{false};
  bool sim_initialized{false};
  bool execute_sim_init{false};
  bool pause_state{true};
  bool mouse_dragging{false};
  bool change_color{true};
  std::unique_ptr<Renderer> renderer;
};

AppState::AppState() {
  renderer = std::make_unique<Renderer>();
  //renderer->simulator->sim_change_pause_state(true);
  //renderer->simulator->sim_set_write_output(false);
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  *appstate = new AppState;
  auto *app = static_cast<AppState *>(*appstate);

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
  // GL ES 2.0 + GLSL 100 (WebGL 1.0)
  const char *glsl_version = "#version 100";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
  // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
  const char *glsl_version = "#version 300 es";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
  // GL 3.2 Core + GLSL 150
  const char *glsl_version = "#version 150";
  SDL_GL_SetAttribute(
      SDL_GL_CONTEXT_FLAGS,
      SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
  // GL 3.0 + GLSL 130
  const char *glsl_version = "#version 130";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

  // Create window with graphics context
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  SDL_WindowFlags window_flags =
      SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;

  app->window_ptr = SDL_CreateWindow("ImGui+SDL3+OpenGL3", 1280,
                                     800, window_flags);
  app->context_ptr = SDL_GL_CreateContext(app->window_ptr);

  if (app->window_ptr == nullptr) {
    printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (app->context_ptr == nullptr) {
    printf("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  SDL_GL_MakeCurrent(app->window_ptr, app->context_ptr);
  SDL_GL_SetSwapInterval(-1); // Enable adaptive vsync
  SDL_SetWindowPosition(app->window_ptr, SDL_WINDOWPOS_CENTERED,
                        SDL_WINDOWPOS_CENTERED);
  SDL_ShowWindow(app->window_ptr);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();

  // Setup scaling
  ImGuiStyle &style = ImGui::GetStyle();
  float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
  style.ScaleAllSizes(
      main_scale); // Bake a fixed style scale. (until we have a solution for
                   // dynamic style scaling, changing this requires resetting
                   // Style + calling this again)
  style.FontScaleDpi =
      main_scale; // Set initial font scale. (using io.ConfigDpiScaleFonts=true
                  // makes this unnecessary. We leave both here for
                  // documentation purpose)

  // Setup Platform/Renderer backends
  ImGui_ImplSDL3_InitForOpenGL(app->window_ptr, app->context_ptr);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Initialize OpenGL extensions
  glewInit();

  return SDL_APP_CONTINUE;
}

// This function runs when a new event (mouse input, keypresses, etc) occurs.
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *ev) {
  auto *app = static_cast<AppState *>(appstate);

  ImGui_ImplSDL3_ProcessEvent(ev);

  // Quit if escape key pressed, window is closed, or termination signal is
  // received
  if ((ev->type == SDL_EVENT_KEY_DOWN && ev->key.key == SDLK_ESCAPE) ||
      (ev->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
       ev->window.windowID == SDL_GetWindowID(app->window_ptr)) ||
      (ev->type == SDL_EVENT_QUIT)) {

    // Sim seg faults if forced to quit before initialization
    //if (!app->sim_initialized) {
    //  app->renderer->simulator->initialize_simulation(1.0f, 1, 1, 1, 1.0f);
    //}

    // Stop and exit
    app->app_finished = true;
    return SDL_APP_SUCCESS;
  }

  ImGuiIO &io = ImGui::GetIO();

  // Handle mouse inputs
  switch (ev->type) {

  case SDL_EVENT_MOUSE_BUTTON_DOWN:
    if (ev->button.button == SDL_BUTTON_LEFT) {
      app->mouse_dragging = true;

      if (!io.WantCaptureMouse) {
        app->renderer->camera.start_drag(
            glm::vec2(ev->button.x, ev->button.y));
      }
    }
    break;

  case SDL_EVENT_MOUSE_MOTION:
    if (app->mouse_dragging) {

      if (!io.WantCaptureMouse) {
        int w, h;
        SDL_GetWindowSizeInPixels(app->window_ptr, &w, &h);
        app->renderer->camera.update_drag(
            glm::vec2(ev->motion.x, ev->motion.y),
            glm::vec2(w, h));
      }
    }
    break;

  case SDL_EVENT_MOUSE_BUTTON_UP:
    if (ev->button.button == SDL_BUTTON_LEFT) {
      app->mouse_dragging = false;

      if (!io.WantCaptureMouse) {
        app->renderer->camera.end_drag();
      }
    }
    break;

  case SDL_EVENT_MOUSE_WHEEL:
    if (!io.WantCaptureMouse) {
      app->renderer->camera.update_zoom(ev->wheel.y);
    }
    break;

  }


  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  auto *app = static_cast<AppState *>(appstate);

  if (SDL_GetWindowFlags(app->window_ptr) & SDL_WINDOW_MINIMIZED) {
    SDL_Delay(10);
  }

  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  ImGuiIO &io = ImGui::GetIO();

  // Get input from user
  static int NBODS = 32768;
  static float DTIME = 10.0f;
  static Color::ColorType COLOR;
  {
    ImGui::Begin("INPUTS");
    ImGui::InputInt("Number of particles", &NBODS);
    ImGui::InputFloat("Timestep", &DTIME);

    ImGui::SeparatorText("CONTROLS");
    if (ImGui::Button("INITIALIZE")) {
      app->execute_sim_init = true;
    }
    ImGui::SameLine();

    if (ImGui::Button("RUN")) {
      app->pause_state = false;
      //app->renderer->simulator->sim_change_pause_state(app->pause_state);
      app->execute_sim_init = true;
    }
    ImGui::SameLine();

    //if (ImGui::Button("PAUSE")) {
    //  app->pause_state = true;
    //  //app->renderer->simulator->sim_change_pause_state(app->pause_state);
    //}
    //ImGui::SameLine();

    //if (ImGui::Button("RESET")) {
    //  if (app->sim_initialized) {
    //    app->pause_state = true;
    //    app->execute_sim_init = false;
    //    app->sim_initialized = false;
    //    app->change_color = true;
    //    app->renderer->reset_simulator();
    //    app->renderer->simulator->sim_change_pause_state(app->pause_state);
    //    app->renderer->simulator->sim_set_write_output(false);
    //  }
    //}
    //ImGui::SameLine();

    if (ImGui::Button("QUIT")) {
      SDL_Event sdl_quit;
      sdl_quit.type = SDL_EVENT_QUIT;
      SDL_PushEvent(&sdl_quit);
    }
    ImGui::SameLine();


    ImGui::ColorEdit3("Clear color", (float*)&app->clear_color);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / io.Framerate, io.Framerate);

    ImGui::End();
  }

  

  // Rendering
  ImGui::Render();

  int w, h;
  SDL_GetWindowSizeInPixels(app->window_ptr, &w, &h);
  glViewport(0, 0, w, h);

  glClearColor( app->clear_color.x * app->clear_color.w,
                app->clear_color.y * app->clear_color.w,
                app->clear_color.z * app->clear_color.w,
                app->clear_color.w);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Initialize renderer, simulation, camera
  if (app->execute_sim_init && !app->sim_initialized) {
    app->renderer->init(NBODS);
    app->sim_initialized = true;
  }

  // Change color palette if user changes
 // if (app->sim_initialized && app->change_color) {
 //   app->renderer->change_color(COLOR);
 //   app->change_color = false;
 // }

  if (app->sim_initialized) {

    // Run simulation and update data buffers
    app->renderer->update(DTIME);

    // Display simulation data
    float aspect_ratio = static_cast<float>(w) / static_cast<float>(h);
    app->renderer->display(aspect_ratio);
  }

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  SDL_GL_SwapWindow(app->window_ptr);

  return SDL_APP_CONTINUE; /* carry on with the program! */
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();

  if (appstate != NULL) {
    auto *app = static_cast<AppState *>(appstate);
    SDL_GL_DestroyContext(app->context_ptr);
    SDL_DestroyWindow(app->window_ptr);
    delete app;
  }
}
