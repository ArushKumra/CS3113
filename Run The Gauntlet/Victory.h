#include "Scene.h"

class Victory : public Scene {
public:
    int ENEMY_COUNT = 3;

    ~Victory();

    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
};
