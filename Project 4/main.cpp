/**
* Author: Arush Kumra
* Assignment: Rise of the AI
* Date due: 2024-07-27, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define PLATFORM_COUNT 11
#define TOTAL_PLATFORMS 20
#define ENEMY_COUNT 3

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"

// ––––– STRUCTS AND ENUMS ––––– //
struct GameState
{
    Entity *player;
    Entity *platforms;
    Entity *enemies;
    
    Mix_Music *bgm;
    Mix_Chunk *jump_sfx;
};

enum AppStatus { RUNNING, TERMINATED };

// ––––– CONSTANTS ––––– //
constexpr int WINDOW_WIDTH  = 1280,
          WINDOW_HEIGHT = 960;

constexpr float BG_RED     = 0.1922f,
            BG_BLUE    = 0.549f,
            BG_GREEN   = 0.9059f,
            BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
          VIEWPORT_Y = -WINDOW_HEIGHT/4,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

constexpr char SPRITESHEET_FILEPATH[] = "assets/spritesheet.png",
           PLATFORM_FILEPATH[]    = "assets/tile_0002.png",
           ENEMY_FILEPATH[]       = "assets/tile_0109.png",
           FONTSHEET_FILEPATH[] = "assets/font1.png";

constexpr char BGM_FILEPATH[] = "assets/Mesmerizing Galaxy Loop.mp3",
           SFX_FILEPATH[] = "assets/cartoon-jump-6462.wav";

constexpr int FONTBANK_SIZE = 16;

constexpr int NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL  = 0;
constexpr GLint TEXTURE_BORDER   = 0;
GLuint g_font_texture_id;

constexpr float PLATFORM_OFFSET = 5.0f;

bool win = false;
bool lose = false;
int kills = 0;

// ––––– VARIABLES ––––– //
GameState g_game_state;

SDL_Window* g_display_window;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

AppStatus g_app_status = RUNNING;

GLuint load_texture(const char* filepath);

void initialise();
void process_input();
void update();
void render();
void shutdown();
void draw_text(ShaderProgram* shader_program, GLuint font_texture_id, std::string text, float font_size, float spacing, glm::vec3 position);

// ––––– GENERAL FUNCTIONS ––––– //
GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    stbi_image_free(image);
    
    return textureID;
}

void initialise()
{
    // ––––– GENERAL STUFF ––––– //
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("Rise of the AI",
                                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  WINDOW_WIDTH, WINDOW_HEIGHT,
                                  SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

    if (context == nullptr)
    {
        LOG("ERROR: Could not create OpenGL context.\n");
        shutdown();
    }

    #ifdef _WINDOWS
    glewInit();
    #endif
    // ––––– VIDEO STUFF ––––– //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());



    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    // ––––– PLATFORM ––––– //
    GLuint platform_texture_id = load_texture(PLATFORM_FILEPATH);
    g_font_texture_id = load_texture(FONTSHEET_FILEPATH);

    g_game_state.platforms = new Entity[TOTAL_PLATFORMS];

    for (int i = 0; i < PLATFORM_COUNT; i++)
    {
    g_game_state.platforms[i] = Entity(platform_texture_id,0.0f, 0.4f, 1.0f, PLATFORM);
    g_game_state.platforms[i].set_position(glm::vec3(i - PLATFORM_OFFSET, -3.0f, 0.0f));
    g_game_state.platforms[i].set_scale(glm::vec3(0.5f));
    g_game_state.platforms[i].update(0.0f, NULL, NULL, 0);
    }


    for (int i = 11; i < TOTAL_PLATFORMS; i++)
    {
        g_game_state.platforms[i] = Entity(platform_texture_id, 0.0f, 0.4f, 1.0f, PLATFORM);
        g_game_state.platforms[i].set_scale(glm::vec3(0.5f));
    }

    for (int i = 11; i < 16; i++)
    {
        g_game_state.platforms[i].set_position(glm::vec3(i - 10.0f, 1.0f, 0.0f));
        g_game_state.platforms[i].update(0.0f, NULL, NULL, 0);
    }
    for (int i = 16; i < 20; i++) {
        g_game_state.platforms[i].set_position(glm::vec3(i - 24.0f, 4.0f, 0.0f));
        g_game_state.platforms[i].update(0.0f, NULL, NULL, 0);
    }

   

    //GEORGE//
    GLuint player_texture_id = load_texture(SPRITESHEET_FILEPATH);

    int player_walking_animation[4][12] =
    {
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 },  // idle,
    { 12 }, // jump
    { 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35 }, // right,
    { 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47 }   // left
    };

    glm::vec3 acceleration = glm::vec3(0.0f,-4.905f, 0.0f);

    g_game_state.player = new Entity(
    player_texture_id,         // texture id
    5.0f,                      // speed
    acceleration,              // acceleration
    3.0f,                      // jumping power
    player_walking_animation,  // animation index sets
    0.0f,                      // animation time
    12,                         // animation frame amount
    0,                         // current animation index
    12,                         // animation column amount
    4,                         // animation row amount
    0.9f,                      // width
    0.9f,                       // height
    PLAYER
    );


    // Jumping
    g_game_state.player->set_jumping_power(7.0f);
    g_game_state.player->set_position(glm::vec3(-3.0f, 0,0));
    g_game_state.player->set_scale(glm::vec3(0.5f));

    

    // ––––– SOPHIE ––––– //
    GLuint enemy_texture_id = load_texture(ENEMY_FILEPATH);

    g_game_state.enemies = new Entity[ENEMY_COUNT];

    for (int i = 0; i < ENEMY_COUNT; i++)
    {
    g_game_state.enemies[i] =  Entity(enemy_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, GUARD, IDLE);
    g_game_state.enemies[i].set_scale(glm::vec3(0.5f));
    }


    g_game_state.enemies[0].set_position(glm::vec3(3.0f, 0.0f, 0.0f));
    g_game_state.enemies[0].set_movement(glm::vec3(0.0f));
    g_game_state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    g_game_state.enemies[1].set_position(glm::vec3(1.0f, 1.0f, 0.0f));
    g_game_state.enemies[1].set_ai_state(PATROLLING);
    g_game_state.enemies[1].set_movement(glm::vec3(0.0f));
    g_game_state.enemies[1].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    g_game_state.enemies[2].set_position(glm::vec3(-6.5f, 4.0f, 0.0f));
    g_game_state.enemies[2].set_jumping_power(2.0f);
    g_game_state.enemies[2].set_ai_state(JUMPING);
    g_game_state.enemies[2].set_movement(glm::vec3(0.0f));
    g_game_state.enemies[2].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));

    // ––––– AUDIO STUFF ––––– //
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    g_game_state.bgm = Mix_LoadMUS(BGM_FILEPATH);
    Mix_PlayMusic(g_game_state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 4.0f);

    g_game_state.jump_sfx = Mix_LoadWAV(SFX_FILEPATH);

    // ––––– GENERAL STUFF ––––– //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
 g_game_state.player->set_movement(glm::vec3(0.0f));
 
 SDL_Event event;
 while (SDL_PollEvent(&event))
 {
     switch (event.type) {
         // End game
         case SDL_QUIT:
         case SDL_WINDOWEVENT_CLOSE:
             g_app_status = TERMINATED;
             break;
             
         case SDL_KEYDOWN:
             switch (event.key.keysym.sym) {
                 case SDLK_q:
                     // Quit the game with a keystroke
                     g_app_status = TERMINATED;
                     break;
                     
                 case SDLK_SPACE:
                     // Jump
                     if (g_game_state.player->get_collided_bottom())
                     {
                         g_game_state.player->jump();
                         Mix_PlayChannel(-1, g_game_state.jump_sfx, 0);
                     }
                     break;
                     
                 default:
                     break;
             }
             
         default:
             break;
     }
 }
 
 const Uint8 *key_state = SDL_GetKeyboardState(NULL);

 if (key_state[SDL_SCANCODE_LEFT])       g_game_state.player->move_left();
 else if (key_state[SDL_SCANCODE_RIGHT]) g_game_state.player->move_right();
 else g_game_state.player->move_down();
     
 if (glm::length(g_game_state.player->get_movement()) > 1.0f)
    g_game_state.player->normalise_movement();
 
 
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    delta_time += g_accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP)
    {
        g_game_state.player->update(FIXED_TIMESTEP, g_game_state.player, g_game_state.platforms, TOTAL_PLATFORMS);

        for (int i = 0; i < ENEMY_COUNT; i++)
            g_game_state.enemies[i].update(FIXED_TIMESTEP,
                                           g_game_state.player,
                                           g_game_state.platforms,
                TOTAL_PLATFORMS);

        delta_time -= FIXED_TIMESTEP;
    }

    g_accumulator = delta_time;
    if (g_game_state.player->get_position().y >= 4.0f) g_game_state.player->set_jumping_power(5.0f);
    else g_game_state.player->set_jumping_power(7.0f);
 
    g_game_state.player->check_collision_x(g_game_state.enemies, ENEMY_COUNT);
    g_game_state.player->check_collision_y(g_game_state.enemies, ENEMY_COUNT);
    if (g_game_state.player->get_killed()) {
        lose = true;
        g_game_state.player->deactivate();
        for (int i = 0; i < ENEMY_COUNT; i++) {
            g_game_state.enemies[i].deactivate();
        }
    }
    else if (g_game_state.player->get_enemy_kill()) {
        kills += 1;
        g_game_state.player->reset_kill();
    }

    if (kills == 3) win = true;
    
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    g_game_state.player->render(&g_shader_program);

    for (int i = 0; i < TOTAL_PLATFORMS; i++)
        g_game_state.platforms[i].render(&g_shader_program);
    for (int i = 0; i < ENEMY_COUNT; i++)
        g_game_state.enemies[i].render(&g_shader_program);
    if (lose) {
        draw_text(&g_shader_program, g_font_texture_id, "YOU LOSE", 1.0f, 0.05f, glm::vec3(-3.5f, 3.0f, 0.0f));
    }
    else if (win) {
        draw_text(&g_shader_program, g_font_texture_id, "YOU WIN!", 1.0f, 0.05f, glm::vec3(-3.5f, 3.0f, 0.0f));
    }
    SDL_GL_SwapWindow(g_display_window);
    
}

void shutdown()
{
    SDL_Quit();

    delete [] g_game_state.platforms;
    delete [] g_game_state.enemies;
    delete    g_game_state.player;
    Mix_FreeChunk(g_game_state.jump_sfx);
    Mix_FreeMusic(g_game_state.bgm);
}

void draw_text(ShaderProgram* shader_program, GLuint font_texture_id, std::string text,
    float font_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

    // Instead of having a single pair of arrays, we'll have a series of pairs—one for
    // each character. Don't forget to include <vector>!
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their
        //    position relative to the whole sentence)
        int spritesheet_index = (int)text[i];  // ascii value of character
        float offset = (font_size + spacing) * i;

        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float)(spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float)(spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * font_size), 0.5f * font_size,
            offset + (-0.5f * font_size), -0.5f * font_size,
            offset + (0.5f * font_size), 0.5f * font_size,
            offset + (0.5f * font_size), -0.5f * font_size,
            offset + (0.5f * font_size), 0.5f * font_size,
            offset + (-0.5f * font_size), -0.5f * font_size,
            });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
            });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);

    shader_program->set_model_matrix(model_matrix);
    glUseProgram(shader_program->get_program_id());

    glVertexAttribPointer(shader_program->get_position_attribute(), 2, GL_FLOAT, false, 0,
        vertices.data());
    glEnableVertexAttribArray(shader_program->get_position_attribute());

    glVertexAttribPointer(shader_program->get_tex_coordinate_attribute(), 2, GL_FLOAT,
        false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(shader_program->get_tex_coordinate_attribute());

    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(shader_program->get_position_attribute());
    glDisableVertexAttribArray(shader_program->get_tex_coordinate_attribute());
}

// ––––– GAME LOOP ––––– //
int main(int argc, char* argv[])
{
    initialise();

    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
