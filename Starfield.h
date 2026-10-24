#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <SDL3/SDL.h>
#include <vector>

// Configuration Constants
const int WINDOW_WIDTH = 3840;   
const int WINDOW_HEIGHT = 2160;  
const int STAR_COUNT = 4000;     
const float MAX_DEPTH = 1000.0f;  

// Speed Boundaries
const float MIN_SPEED = 0.0f;        
const float DEFAULT_SPEED = 50.0f;  
const float MAX_SPEED = 4000.0f;     
const float SPEED_STEP = 25.0f;     

enum StarType {
    STAR_WHITE,
    STAR_LIGHT_BLUE,
    STAR_LIGHT_RED,
    STAR_LIGHT_YELLOW
};

struct Star {
    float x, y, z;      
    float last_screen_x; 
    float last_screen_y;
    StarType type;      
    
    // Twinkling animation properties
    float twinkle_speed;
    float twinkle_offset;
};

inline float Clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

inline float Lerp(float start, float end, float t) {
    return start + t * (end - start);
}
