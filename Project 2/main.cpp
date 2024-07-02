/**
* Author: Arush Kumra
* Assignment: Pong Clone
* Date due: 2024-06-29, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

enum AppStatus { RUNNING, TERMINATED };

constexpr int WINDOW_WIDTH  = 640,
              WINDOW_HEIGHT = 480;

constexpr float BG_RED     = 0.9765625f,
                BG_GREEN   = 0.97265625f,
                BG_BLUE    = 0.9609375f,
                BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X      = 0,
              VIEWPORT_Y      = 0,
              VIEWPORT_WIDTH  = WINDOW_WIDTH,
              VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
               F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

constexpr GLint NUMBER_OF_TEXTURES = 1, // to be generated, that is
                LEVEL_OF_DETAIL    = 0, // mipmap reduction image level
                TEXTURE_BORDER     = 0; // this value MUST be zero


constexpr char PRAY_SPRITE_FILEPATH[]   = "one.png",
               FREY_SPRITE_FILEPATH[] = "two.jpg",
               BALL_SPRITE_FILEPATH[] = "ball.png"; //source: https://www.vecteezy.com/png/26749133-tennis-ball-clipart

constexpr glm::vec3 INIT_SCALE      = glm::vec3(1.0f, 1.0f, 1.0f),
                    INIT_POS_PRAY   = glm::vec3(4.0f, 0.0f, 0.0f),
                    INIT_POS_FREY = glm::vec3(-4.0f, 0.0f, 0.0f),
                    INIT_POS_BALL = glm::vec3(0,0,0);

glm::vec3 g_frey_pos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_pray_pos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_ball_pos = glm::vec3(0.0f, 0.0f, 0.0f);

bool singlePlayer = false;

constexpr float ROT_INCREMENT = 1.0f;
float TRAN_INCREMENT = 0.5f;

SDL_Window* g_display_window;
AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program = ShaderProgram();

glm::mat4 g_view_matrix,
          g_pray_matrix,
          g_frey_matrix,
          g_ball_matrix,
          g_projection_matrix;

float g_previous_ticks = 0.0f;

glm::vec3 g_translation_up = glm::vec3(0.0f, 0.5f, 0.0f),
          g_translation_down = glm::vec3(0.0f, -0.5f, 0.0f);

glm::vec3 g_pray_movement = glm::vec3(0, 0, 0);
glm::vec3 g_frey_movement = glm::vec3(0, 0, 0);
glm::vec3 g_ball_movement = glm::vec3(1.5f, 2.0f, 0);

GLuint g_pray_texture_id,
       g_frey_texture_id,
       g_ball_texture_id;


GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    
    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);
    
    return textureID;
}


void initialise()
{
    // Initialise video and joystick subsystems
    SDL_Init(SDL_INIT_VIDEO);
    
    g_display_window = SDL_CreateWindow("Project 2: Pong Clone",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

    if (g_display_window == nullptr)
    {
        std::cerr << "Error: SDL window could not be created.\n";
        SDL_Quit();
        exit(1);
    }
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_pray_matrix       = glm::mat4(1.0f);
    g_frey_matrix       = glm::mat4(1.0f);
    g_ball_matrix       = glm::mat4(1.0f);
    g_view_matrix       = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    g_pray_texture_id   = load_texture(PRAY_SPRITE_FILEPATH);
    g_frey_texture_id = load_texture(FREY_SPRITE_FILEPATH);
    g_ball_texture_id = load_texture(BALL_SPRITE_FILEPATH);


    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
}


void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                case SDLK_t:
                    singlePlayer = !singlePlayer;
                    g_pray_movement.y = 1.0f;
                    break;

                case SDLK_q:
                    g_app_status = TERMINATED;
                    break;

                default:
                    break;
                }
        }
        const Uint8* key_state = SDL_GetKeyboardState(NULL);

        if (key_state[SDL_SCANCODE_W])
        {
            g_frey_movement.y = 2.0f;
        }

        else if (key_state[SDL_SCANCODE_S])
        {
            g_frey_movement.y = -2.0f;
        }

        else {
            g_frey_movement.y = 0;
        }

        if (key_state[SDL_SCANCODE_UP])
        {
            if (!singlePlayer) {
                g_pray_movement.y = 2.0f;
            } 
        }

        else if (key_state[SDL_SCANCODE_DOWN])
        {
            if (!singlePlayer) {
                g_pray_movement.y = -2.0f;
            }
        }

        else {
            if (!singlePlayer) {
                g_pray_movement.y = 0;
            }
        }
    }
}


void update()
{
    /* Delta time calculations */
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    /* Game logic */
    //g_rotation_pray.y += ROT_INCREMENT * delta_time;


    /* Model matrix reset */
    g_pray_matrix   = glm::mat4(1.0f);
    g_frey_matrix = glm::mat4(1.0f);
    g_ball_matrix = glm::mat4(1.0f);
    
    /* Transformations */
    g_pray_pos += g_pray_movement * 2.0f * delta_time;
    g_pray_matrix = glm::translate(g_pray_matrix, INIT_POS_PRAY);
    g_pray_matrix = glm::translate(g_pray_matrix, g_pray_pos);
    g_pray_matrix = glm::scale(g_pray_matrix, glm::vec3(1.0f, 1.0f, 1.0f));

    g_frey_pos += g_frey_movement * 2.0f * delta_time;
    g_frey_matrix = glm::translate(g_frey_matrix, INIT_POS_FREY);
    g_frey_matrix = glm::translate(g_frey_matrix, g_frey_pos);
    g_frey_matrix = glm::scale(g_frey_matrix, glm::vec3(1.0f, 1.0f, 1.0f));

    g_ball_pos += g_ball_movement * 1.0f * delta_time;
    g_ball_matrix = glm::translate(g_ball_matrix, INIT_POS_BALL);
    g_ball_matrix = glm::translate(g_ball_matrix, g_ball_pos);
    g_ball_matrix = glm::scale(g_ball_matrix, glm::vec3(1.0f, 1.0f, 1.0f));

    /** ———— COLLISION DETECTION ———— **/
    float x_distance_frey = fabs((g_ball_pos.x + INIT_POS_BALL.x) - (g_frey_pos.x + INIT_POS_FREY.x)) - ((INIT_SCALE.x + INIT_SCALE.x) / 2.0f);
    float y_distance_frey = fabs((g_ball_pos.y + INIT_POS_BALL.x) - (g_frey_pos.y + INIT_POS_FREY.y)) - ((INIT_SCALE.y + INIT_SCALE.y) / 2.0f);
    float x_distance_pray = fabs((g_ball_pos.x + INIT_POS_BALL.x) - (g_pray_pos.x + INIT_POS_PRAY.x)) - ((INIT_SCALE.x + INIT_SCALE.x) / 2.0f);
    float y_distance_pray = fabs((g_ball_pos.y + INIT_POS_BALL.y) - (g_pray_pos.y + INIT_POS_PRAY.y)) - ((INIT_SCALE.y + INIT_SCALE.y) / 2.0f);
    if ((x_distance_frey < 0.0f && y_distance_frey < 0.0f) || (x_distance_pray < 0.0f && y_distance_pray < 0.0f))
    {
        g_ball_movement.x *= -1;
        float y_mov = -1.0f + static_cast <float> (rand() % 100)/50;
        g_ball_movement.y = y_mov;
    }
    if (g_ball_pos.y > 3.0f || g_ball_pos.y < -3.0f) {
        g_ball_movement.y *= -1;
    }
    if (singlePlayer) {
        if (g_pray_pos.y > 3.0f || g_pray_pos.y < -3.0f) {
            g_pray_movement.y *= -1;
        }
    }
    if (g_ball_pos.x < -4.5f) {
        std::cout << "Pray won" << std::endl;
        g_app_status = TERMINATED;
    }
    else if (g_ball_pos.x > 4.5f) {
        std::cout << "Frey won" << std::endl;
        g_app_status = TERMINATED;
    }
}


void draw_object(glm::mat4 &object_g_model_matrix, GLuint &object_texture_id)
{
    g_shader_program.set_model_matrix(object_g_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so use 6, not 3
}


void render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Vertices
    float vertices[] =
    {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates[] =
    {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };
    
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false,
                          0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT,
                          false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    // Bind texture
    draw_object(g_pray_matrix, g_pray_texture_id);
    draw_object(g_frey_matrix, g_frey_texture_id);
    draw_object(g_ball_matrix, g_ball_texture_id);
    
    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    SDL_GL_SwapWindow(g_display_window);
}


void shutdown() { SDL_Quit(); }


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

