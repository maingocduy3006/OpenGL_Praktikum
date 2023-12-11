#pragma once

#include "OpenGLWindow.h"
#include "Transform.h"
#include <ShaderProgram.h>
#include <memory>
#include <AssetManager.h>


struct Spotlight {
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 color;
    float intensity;
    float phi;    // inner cone angle in radians
    float gamma;  // outer cone angle in radians
    float constant;
    float linear;
    float quadratic;
};

class Scene
{
public:
    Scene(OpenGLWindow* window);
    ~Scene();

    bool init();
    void shutdown();
    void render(float dt);
    void update(float dt);
    void initFloor();
    OpenGLWindow* getWindow();

    void onKey(Key key, Action action, Modifier modifier);
    void onMouseMove(MousePosition mouseposition);
    void onMouseButton(MouseButton button, Action action, Modifier modifier);
    void onMouseScroll(double xscroll, double yscroll);
    void onFrameBufferResize(int width, int height);

private:
    OpenGLWindow* m_window;
    AssetManager m_assets;
    ShaderProgram* m_shader;
    GLuint vaoID, vboID;
    GLuint  iboID;
    glm::vec3 cameraPos, cameraTarget, cameraUp;
    glm::mat4 views, projection;
    glm::vec3 lightPosition, lightColor, ambientLight, materialSpecular;
    float lightIntensity, materialShininess;
    Transform *head, *body, *rightUpperArm, *leftUpperArm, *rightLowerArm, *leftLowerArm, *rightArm, *leftArm, *rightLeg, *leftLeg, *world, *robot;
    Transform *lightCube, *floor;
    Spotlight spotlight;
};

