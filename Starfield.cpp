#ifdef _WIN32
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

#include "Starfield.h"
#include <SDL3/SDL_main.h>
#include <random>
#include <cmath>
#include <string>

int main(int argc, char* argv[]) {
    bool isPreview = false;
    HWND parentHwnd = nullptr;

    if (argc > 1) {
        std::string flag = argv[1];
        if (flag.substr(0, 2) == "/c" || flag.substr(0, 2) == "-c") {
            MessageBoxA(nullptr, "Starfield Screensaver\nSpeed controls: Mouse Wheel\nExit: Move Mouse or press ESC.", "Settings", MB_OK | MB_ICONINFORMATION);
            return 0;
        }
        else if (flag.substr(0, 2) == "/p" || flag.substr(0, 2) == "-p") {
            isPreview = true;
            if (argc > 2) {
                parentHwnd = (HWND)std::stoull(argv[2]);
            }
        }
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        return -1;
    }

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (isPreview && parentHwnd != nullptr) {
        RECT rect;
        GetClientRect(parentHwnd, &rect);
        
        SDL_PropertiesID props = SDL_CreateProperties();
        SDL_SetPointerProperty(props, "win32.hwnd", parentHwnd);
        SDL_SetNumberProperty(props, "width", rect.right - rect.left);
        SDL_SetNumberProperty(props, "height", rect.bottom - rect.top);
        SDL_SetBooleanProperty(props, "borderless", true);
        
        window = SDL_CreateWindowWithProperties(props);
    } else {
        window = SDL_CreateWindow("Starfield Screensaver", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
        if (window) {
            SDL_SetWindowFullscreen(window, true);
        }
    }

    if (!window) {
        SDL_Quit();
        return -1;
    }

    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    SDL_SetRenderVSync(renderer, 1);
    SDL_HideCursor();

    int current_w, current_h;
    SDL_GetWindowSize(window, &current_w, &current_h);
    float centerX = current_w / 2.0f;
    float centerY = current_h / 2.0f;

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_real_distribution<float> dist_xy(-1200.0f, 1200.0f);
    std::uniform_real_distribution<float> dist_z(10.0f, MAX_DEPTH);
    std::uniform_real_distribution<float> dist_color_roll(0.0f, 1.0f);
    
    // Setup random variance pools for twinkle loops
    std::uniform_real_distribution<float> dist_twinkle_speed(1.0f, 2.0f);
    std::uniform_real_distribution<float> dist_twinkle_offset(0.0f, 6.28f);

    std::vector<Star> stars(STAR_COUNT);
    for (int i = 0; i < STAR_COUNT; ++i) {
        stars[i].x = dist_xy(generator);
        stars[i].y = dist_xy(generator);
        stars[i].z = dist_z(generator);
        stars[i].last_screen_x = -1.0f;
        stars[i].last_screen_y = -1.0f;
        stars[i].twinkle_speed = dist_twinkle_speed(generator);
        stars[i].twinkle_offset = dist_twinkle_offset(generator);

        float roll = dist_color_roll(generator);
        if (roll < 0.85f) {
            stars[i].type = STAR_WHITE;        
        } else if (roll < 0.90f) {
            stars[i].type = STAR_LIGHT_BLUE;   
        } else if (roll < 0.95f) {
            stars[i].type = STAR_LIGHT_RED;    
        } else {
            stars[i].type = STAR_LIGHT_YELLOW; 
        }
    }

    bool running = true;
    Uint64 last_time = SDL_GetTicks();
    float global_time = 0.0f;
    float current_speed = DEFAULT_SPEED;
    float target_speed = DEFAULT_SPEED;
    float start_mouse_x = -1.0f, start_mouse_y = -1.0f;
    // Continues right inside main from Part 1
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            else if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                if (!isPreview) running = false; 
            }
            else if (event.type == SDL_EVENT_MOUSE_MOTION) {
                if (!isPreview) {
                    if (start_mouse_x == -1.0f) {
                        start_mouse_x = event.motion.x;
                        start_mouse_y = event.motion.y;
                    }
                    if (std::abs(event.motion.x - start_mouse_x) > 4.0f || std::abs(event.motion.y - start_mouse_y) > 4.0f) {
                        running = false;
                    }
                }
            }
            else if (event.type == SDL_EVENT_MOUSE_WHEEL) {
                if (event.wheel.y > 0.0f) target_speed += SPEED_STEP;
                else if (event.wheel.y < 0.0f) target_speed -= SPEED_STEP;
                target_speed = Clamp(target_speed, MIN_SPEED, MAX_SPEED);
            }
        }

        Uint64 current_time = SDL_GetTicks();
        float delta_time = (current_time - last_time) / 1000.0f;
        last_time = current_time;
        global_time += delta_time;

        current_speed = Lerp(current_speed, target_speed, 8.0f * delta_time);

        SDL_SetRenderDrawColor(renderer, 5, 15, 25, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255,255,255,255);
        SDL_RenderDebugTextFormat(renderer, 10.0f, 10.0f, "time: %f", global_time);
        if (global_time > 1.0f){

            SDL_SetRenderDrawColor(renderer, global_time, global_time, global_time, SDL_ALPHA_OPAQUE);
            SDL_RenderClear(renderer);
        }
      

        for (int i = 0; i < STAR_COUNT; ++i) {
            stars[i].z -= current_speed * delta_time;

            if (stars[i].z <= 0.0f) {
                stars[i].x = dist_xy(generator);
                stars[i].y = dist_xy(generator);
                stars[i].z = MAX_DEPTH;
                stars[i].last_screen_x = -1.0f;
                stars[i].last_screen_y = -1.0f;
                stars[i].twinkle_speed = dist_twinkle_speed(generator);
                stars[i].twinkle_offset = dist_twinkle_offset(generator);
                
                float roll = dist_color_roll(generator);
                if (roll < 0.85f) stars[i].type = STAR_WHITE;
                else if (roll < 0.90f) stars[i].type = STAR_LIGHT_BLUE;
                else if (roll < 0.95f) stars[i].type = STAR_LIGHT_RED;
                else stars[i].type = STAR_LIGHT_YELLOW;
            }

            float fov_factor = isPreview ? 40.0f : 300.0f; 
            float screen_x = centerX + (stars[i].x / stars[i].z) * fov_factor;
            float screen_y = centerY + (stars[i].y / stars[i].z) * fov_factor;

            if (screen_x >= 0 && screen_x < current_w && screen_y >= 0 && screen_y < current_h) {
                float depth_ratio = (MAX_DEPTH - stars[i].z) / MAX_DEPTH;
                float base_brightness = Clamp(depth_ratio * 1.2f, 0.1f, 1.0f);
                
                // --- STEP 1: CALCULATE THE DYNAMIC TWINKLE WAVE FACTOR ---
                float twinkle_wave = std::sin(global_time * stars[i].twinkle_speed + stars[i].twinkle_offset);
                float twinkle_factor = 0.7f + 0.3f * twinkle_wave; // Scales brightness between 40% and 100%
                
                // Combine distance drop-off with active twinkle pulsation modulation
                float brightness = base_brightness * twinkle_factor;

                float base_r = 1.0f, base_g = 1.0f, base_b = 1.0f;
                switch (stars[i].type) {
                    case STAR_LIGHT_BLUE:   base_r = 0.65f; base_g = 0.85f; base_b = 1.00f; break; 
                    case STAR_LIGHT_RED:    base_r = 1.00f; base_g = 0.40f; base_b = 0.20f; break; 
                    case STAR_LIGHT_YELLOW: base_r = 1.00f; base_g = 0.95f; base_b = 0.60f; break; 
                    case STAR_WHITE:
                    default:                base_r = 1.00f; base_g = 1.00f; base_b = 1.00f; break;
                }
                
                SDL_SetRenderDrawColorFloat(renderer, base_r * brightness, base_g * brightness, base_b * brightness, 1.0f);
                
                if (stars[i].last_screen_x != -1.0f && current_speed > 5.0f) {
                    SDL_RenderLine(renderer, screen_x, screen_y, stars[i].last_screen_x, stars[i].last_screen_y);
                } else {
                    SDL_RenderPoint(renderer, screen_x, screen_y);
                }

                stars[i].last_screen_x = screen_x;
                stars[i].last_screen_y = screen_y;
            } else {
                stars[i].last_screen_x = -1.0f;
                stars[i].last_screen_y = -1.0f;
            }
        }

        if (!isPreview) {
            //SDL_SetRenderDrawColor(renderer, 0, 255, 100, SDL_ALPHA_OPAQUE); 
            //std::string speed_hud = "VELOCITY: " + std::to_string(static_cast<int>(current_speed)) + " LY/S";
            //SDL_RenderDebugText(renderer, 50.0f, 50.0f, speed_hud.c_str());
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    ExitProcess(0); 
    return 0;
}
