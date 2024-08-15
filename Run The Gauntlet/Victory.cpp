#include "Victory.h"
#include "Utility.h"

#define LEVEL_WIDTH 9
#define LEVEL_HEIGHT 8

constexpr char SPRITESHEET_FILEPATH[] = "assets/char.png",
ENEMYA_FILEPATH[] = "assets/tile_0109.png",
ENEMYB_FILEPATH[] = "assets/tile_0120.png";


unsigned int VICTORY_DATA[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 86, 73, 67, 84, 79, 82, 89,
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0,
};

Victory::~Victory()
{
    delete[] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void Victory::initialise()
{
    m_game_state.next_scene_id = -1;

    GLuint map_texture_id = Utility::load_texture("assets/font1.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, VICTORY_DATA, map_texture_id, 1.0f, 16, 16);

    // Code from main.cpp's initialise()
    /**
     George's Stuff
     */
     // Existing
    int player_walking_animation[4][4] =
    {
        { 0, 1, 2, 3 },  // for George to move to the left,
        { 4, 5, 6, 7 }, // for George to move to the right,
        { 8, 9, 10, 11 }, // for George to move upwards,
        { 12, 13, 14, 15 }   // for George to move downwards
    };

    glm::vec3 acceleration = glm::vec3(0.0f, -4.81f, 0.0f);

    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);

    m_game_state.player = new Entity(
        player_texture_id,         // texture id
        5.0f,                      // speed
        acceleration,              // acceleration
        5.0f,                      // jumping power
        player_walking_animation,  // animation index sets
        0.0f,                      // animation time
        4,                         // animation frame amount
        0,                         // current animation index
        4,                         // animation column amount
        4,                         // animation row amount
        1.0f,                      // width
        1.0f,                       // height
        PLAYER
    );

    m_game_state.player->set_position(glm::vec3(0, 0.0f, 0.0f));

    // Jumping
    m_game_state.player->set_jumping_power(4.5f);


    /**
Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture(ENEMYA_FILEPATH);

    m_game_state.enemies = new Entity[ENEMY_COUNT];

    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        m_game_state.enemies[i] = Entity(enemy_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, GUARD, IDLE);
    }


    m_game_state.enemies[0].set_position(glm::vec3(8.0f, 0.0f, 0.0f));
    m_game_state.enemies[0].set_movement(glm::vec3(0.0f));
    m_game_state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));



    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    m_game_state.bgm = Mix_LoadMUS("assets/bgm.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(0.0f);;

    m_game_state.g_victory_sfx = Mix_LoadWAV("assets/victory.wav");

    Mix_PlayChannel(
        -1,       // using the first channel that is not currently in use...
        m_game_state.g_victory_sfx,  // ...play this chunk of audio...
        0        // ...once.
    );

    m_game_state.jump_sfx = Mix_LoadWAV("assets/cartoon-jump-6462.wav");
}

void Victory::update(float delta_time)
{
    //if (m_game_state.player->get_position().y < -2.0f) m_game_state.player->set_movement(glm::vec3(0.5f, 0.0f, 0.0f));
    //else if (m_game_state.player->get_position().x > 0) m_game_state.player->set_movement(glm::vec3(0.5f, 0.0f, 0.0f));
    //m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT, m_game_state.map);

    const Uint8* key_state = SDL_GetKeyboardState(NULL);
    if (key_state[SDL_SCANCODE_RETURN]) {
        m_game_state.next_scene_id = 0;
    }
}

void Victory::render(ShaderProgram* program)
{
    m_game_state.map->render(program);
}
