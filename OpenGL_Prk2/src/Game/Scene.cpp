#include "Scene.h"
#include "Cube.h"
#include <AssetManager.h>

const float d = 3.0f;

Scene::Scene(OpenGLWindow * window) :
	m_window(window)
{
	assert(window != nullptr);
    cameraPos = glm::vec3(-1.0f, 1.0f, 1.0f);
    cameraTarget = glm::vec3(1.0f, -1.0f, -1.0f);
    cameraUp = glm::vec3(0, 1, 0);
    views = glm::lookAt(cameraPos, cameraTarget, cameraUp);

    float theta = 90.0f;
    float aspect = (float)m_window->getWindowWidth()/(float)getWindow()->getWindowHeight();
    float near = 0.1f;
    float far = 100.0f;
    projection = glm::perspective(theta,  aspect,  near, far);

    lightPosition = glm::vec3(0.0, 1.0, 0.0); // Default position of the light
    lightColor = glm::vec3(1.0, 1.0, 1.0);    // White light
    lightIntensity = 1.0f;
    // Full intensity
    ambientLight = glm::vec3(1.0f, 1.0f, 1.0f);
    materialSpecular = glm::vec3(1.0f, 1.0f, 1.0f); // Specular reflection typically white
    materialShininess = 64.0f;                    // Shininess factor

    spotlight.position = glm::vec3(-1.0f, 1.0f, 0.0f);
    spotlight.direction = glm::normalize(glm::vec3(1.0f, -1.0f, 0.0f));  // Pointing diagonally downward
    spotlight.color = glm::vec3(1.0f, 1.0f, 1.0f);  // White light
    spotlight.intensity = 0.2f;
    spotlight.phi = glm::radians(10.0f);   // Inner cone angle
    spotlight.gamma = glm::radians(15.0f); // Outer cone angle
    spotlight.constant = 0.5f;
    spotlight.linear = 0.05f;
    spotlight.quadratic = 0.01f;
}

Scene::~Scene()
{}


bool Scene::init()
{
	try
	{
		//Load shader
		m_assets.addShaderProgram("shader", AssetManager::createShaderProgram("assets/shaders/vertex.glsl", "assets/shaders/fragment.glsl"));
		m_shader = m_assets.getShaderProgram("shader");
        m_shader->use();

        // setup VAO
        glGenVertexArrays(1, &vaoID);
        glBindVertexArray(vaoID);

        // setup VBO
        glGenBuffers(1, &vboID);
        glBindBuffer(GL_ARRAY_BUFFER, vboID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVert), &cubeVert, GL_STATIC_DRAW);

        //robot
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), 0);
        glEnableVertexAttribArray(0);

        //normals
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        //color
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        // setup IBO
        glGenBuffers(1, &iboID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeInd), cubeInd, GL_STATIC_DRAW);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // globale Rotation um Y-Achse, wird jeden Frame gesetzt
        world = new Transform;

        floor = new Transform;
        floor->scale(glm::vec3(10.0f, 0.1f, 10.0f));
        floor->translate(glm::vec3(0.0f, -0.55f, 0.0f));

        lightCube = new Transform;
        lightCube->scale(glm::vec3(0.1f, 0.1f, 0.1f));
        lightCube->setPosition(lightPosition);

        // Einmalige Skalierung von allen
        robot = new Transform;
        robot->scale(glm::vec3(0.1f, 0.1f, 0.1f));

        // Einmalige Skalierung des Rumpfes
        body = new Transform;
        body->scale(glm::vec3(2.0f, 3.5f, 2.1f));

        // fixe Translation des Kopfes
        head = new Transform;
        head->translate(glm::vec3(0, 2.5f, 0));

        leftLeg = new Transform;
        leftLeg->scale(glm::vec3(0.5f, 2.5f, 0.6f));
        leftLeg->translate(glm::vec3(0.7f, -3.2f, 0));

        rightLeg = new Transform;
        rightLeg->scale(glm::vec3(0.5f, 2.5f, 0.6f));
        rightLeg->translate(glm::vec3(-0.7f, -3.2f, 0));

        leftUpperArm = new Transform;
        leftUpperArm->scale(glm::vec3(0.6f, 1.5f, 0.6f));
        leftUpperArm->translate(glm::vec3(-1.5f, 0.6f, 0.0f));

        leftLowerArm = new Transform;
        leftLowerArm->rotateAroundPoint(glm::vec3(0, -0.5, -0.2), glm::vec3(-0.7, 0, 0));
        leftLowerArm->scale(glm::vec3(0.6f, 1.5f, 0.6f));
        leftLowerArm->translate(glm::vec3(-1.5f, -1.15f, 0.0f));

        //for movement of the whole arm
        leftArm = new Transform;
        rightArm = new Transform;

        rightLowerArm = new Transform;
        rightLowerArm->rotateAroundPoint(glm::vec3(0, -0.5, -0.2), glm::vec3(-0.7, 0, 0));
        rightLowerArm->scale(glm::vec3(0.6f, 1.5f, 0.6f));
        rightLowerArm->translate(glm::vec3(1.5f, -1.15f, 0.0f));

        rightUpperArm = new Transform;
        rightUpperArm->scale(glm::vec3(0.6f, 1.5f, 0.6f));
        rightUpperArm->translate(glm::vec3(1.5f, 0.6f, 0.0f));

        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        glCullFace(GL_BACK);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glClearDepth(1.0);

        std::cout << "Scene initialization done\n";
        return true;
	}
	catch (std::exception& ex)
	{
	    throw std::logic_error("Scene initialization failed:\n" + std::string(ex.what()) + "\n");
	}


}

void Scene::render(float dt)
{
    // Clear background color and depth buffer for depth test
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(vaoID);

    static float time = 2;
    static bool rightMove = true;
    time += dt;
    if (time > 4) {
        time = 0;
        rightMove = !rightMove;
    }
    static float c = 0;
    c+=dt;




    robot->rotate(glm::vec3(0, 0.2 * dt, 0));
    m_shader->setUniform("views", views, false);
    m_shader->setUniform("projection", projection, false);


    /*m_shader->setUniform("light.position", lightPosition);
    m_shader->setUniform("light.color", lightColor);
    m_shader->setUniform("light.intensity", lightIntensity);
     */
    m_shader->setUniform("globalAmbient", ambientLight);
    m_shader->setUniform("material.specular", materialSpecular);
    m_shader->setUniform("material.shininess", materialShininess);

    m_shader->setUniform("spotlight.position", spotlight.position);
    m_shader->setUniform("spotlight.direction", spotlight.direction);
    m_shader->setUniform("spotlight.color", spotlight.color);
    m_shader->setUniform("spotlight.intensity", spotlight.intensity);
    m_shader->setUniform("spotlight.phi", spotlight.phi);
    m_shader->setUniform("spotlight.gamma", spotlight.gamma);
    m_shader->setUniform("spotlight.constant", spotlight.constant);
    m_shader->setUniform("spotlight.linear", spotlight.linear);
    m_shader->setUniform("spotlight.quadratic", spotlight.quadratic);

    m_shader->setUniform("mm",   world->getTransformMatrix()*floor->getTransformMatrix(), false);
    glDrawElements(GL_TRIANGLES, sizeof(cubeInd), GL_UNSIGNED_INT, 0);

    m_shader->setUniform("mm", world->getTransformMatrix() * robot->getTransformMatrix() * head->getTransformMatrix(), false);
    glDrawElements(GL_TRIANGLES, sizeof(cubeInd), GL_UNSIGNED_INT, 0);


    m_shader->setUniform("mm", world->getTransformMatrix() * robot->getTransformMatrix() * body->getTransformMatrix(), false);
    glDrawElements(GL_TRIANGLES, sizeof(cubeInd), GL_UNSIGNED_INT, 0);

    m_shader->setUniform("mm", world->getTransformMatrix() * lightCube->getTransformMatrix(), false);
    glDrawElements(GL_TRIANGLES, sizeof(cubeInd), GL_UNSIGNED_INT, 0);

    if (rightMove) {
        leftArm->rotateAroundPoint(glm::vec3(0, 0.3, 0), glm::vec3(-0.2 * dt, 0, 0));
    }
    else {
        leftArm->rotateAroundPoint(glm::vec3(0, 0.3, 0), glm::vec3(0.2 * dt, 0, 0));
    }

    m_shader->setUniform("mm", world->getTransformMatrix() * robot->getTransformMatrix() * leftArm->getTransformMatrix() * leftUpperArm->getTransformMatrix(), false);
    glDrawElements(GL_TRIANGLES, sizeof(cubeInd), GL_UNSIGNED_INT, 0);

    m_shader->setUniform("mm", world->getTransformMatrix() * robot->getTransformMatrix() * leftArm->getTransformMatrix() * leftLowerArm->getTransformMatrix(), false);
    glDrawElements(GL_TRIANGLES, sizeof(cubeInd), GL_UNSIGNED_INT, 0);

    if (rightMove) {
        rightArm->rotateAroundPoint(glm::vec3(0, 0.3, 0), glm::vec3(0.2 * dt, 0, 0));
    } else {
        rightArm->rotateAroundPoint(glm::vec3(0, 0.3, 0), glm::vec3(-0.2 * dt, 0, 0));
    }

    m_shader->setUniform("mm", world->getTransformMatrix() * robot->getTransformMatrix() * rightArm->getTransformMatrix() * rightLowerArm->getTransformMatrix(), false);
    glDrawElements(GL_TRIANGLES, sizeof(cubeInd), GL_UNSIGNED_INT, 0);

    m_shader->setUniform("mm", world->getTransformMatrix() * robot->getTransformMatrix() * rightArm->getTransformMatrix() * rightUpperArm->getTransformMatrix(), false);
    glDrawElements(GL_TRIANGLES, sizeof(cubeInd), GL_UNSIGNED_INT, 0);

    if (rightMove) {
        leftLeg->rotateAroundPoint(glm::vec3(0, -0.3, 0), glm::vec3(-0.1 * dt, 0, 0));
    } else {
        leftLeg->rotateAroundPoint(glm::vec3(0, -0.3, 0), glm::vec3(0.1 * dt, 0, 0));
    }

    m_shader->setUniform("mm", world->getTransformMatrix() * robot->getTransformMatrix() * leftLeg->getTransformMatrix(), false);
    glDrawElements(GL_TRIANGLES, sizeof(cubeInd), GL_UNSIGNED_INT, 0);

    if (rightMove) {
        rightLeg->rotateAroundPoint(glm::vec3(0, -0.3, 0), glm::vec3(0.1 * dt, 0, 0));
    } else {
        rightLeg->rotateAroundPoint(glm::vec3(0, -0.3, 0), glm::vec3(-0.1 * dt, 0, 0));
    }

    m_shader->setUniform("mm", world->getTransformMatrix() * robot->getTransformMatrix() * rightLeg->getTransformMatrix(), false);
    glDrawElements(GL_TRIANGLES, sizeof(cubeInd), GL_UNSIGNED_INT, 0);


    glBindVertexArray(0);
}

void Scene::update(float dt)
{
    // Camera movement
    float cameraSpeed = 0.03f;
    if (m_window->getInput().getKeyState(Key::D) == KeyState::Pressed) {
        // Move the camera to the right along the X-axis
        cameraPos.x += cameraSpeed;
    }
    if (m_window->getInput().getKeyState(Key::A) == KeyState::Pressed) {
        // Move the camera to the left along the X-axis
        cameraPos.x -= cameraSpeed;
    }

    if (m_window->getInput().getKeyState(Key::W) == KeyState::Pressed) {
        // Move the camera forward along the Y-axis
        cameraPos.y += cameraSpeed;
    }
    if (m_window->getInput().getKeyState(Key::S) == KeyState::Pressed) {
        // Move the camera backward along the Y-axis
        cameraPos.y -= cameraSpeed;
    }

    if (m_window->getInput().getKeyState(Key::Q) == KeyState::Pressed) {
        // Move the camera upward along the Z-axis
        cameraPos.z += cameraSpeed;
    }
    if (m_window->getInput().getKeyState(Key::E) == KeyState::Pressed) {
        // Move the camera downward along the Z-axis
        cameraPos.z -= cameraSpeed;
    }

    // Update the view matrix based on the new camera position
    views = glm::lookAt(cameraPos, cameraTarget, cameraUp);
    float lightSpeed = 0.03f;

    if (m_window->getInput().getKeyState(Key::Up) == KeyState::Pressed) {
        spotlight.position.y += lightSpeed;
    }
    if (m_window->getInput().getKeyState(Key::Down) == KeyState::Pressed) {
        spotlight.position.y -= lightSpeed;
    }
    if (m_window->getInput().getKeyState(Key::Left) == KeyState::Pressed) {
        spotlight.position.x -= lightSpeed;
    }
    if (m_window->getInput().getKeyState(Key::Right) == KeyState::Pressed) {
        spotlight.position.x += lightSpeed;
    }
    if (m_window->getInput().getKeyState(Key::Comma) == KeyState::Pressed) {
        spotlight.position.z -= lightSpeed;

    }
    if (m_window->getInput().getKeyState(Key::Period) == KeyState::Pressed) {
        spotlight.position.z += lightSpeed;
    }
    lightCube->setPosition(spotlight.position);
}


OpenGLWindow * Scene::getWindow()
{
	return m_window;
}

void Scene::onKey(Key key, Action action, Modifier modifier)
{

}

void Scene::onMouseMove(MousePosition mouseposition)
{
}


void Scene::onMouseButton(MouseButton button, Action action, Modifier modifier)
{

}

void Scene::onMouseScroll(double xscroll, double yscroll)
{

}

void Scene::onFrameBufferResize(int width, int height)
{

}
void Scene::shutdown()
{

}

