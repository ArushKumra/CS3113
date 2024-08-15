#include "Scene.h"

class Defeat : public Scene {
public:
    int ENEMY_COUNT = 3;

    ~Defeat();

    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
};
