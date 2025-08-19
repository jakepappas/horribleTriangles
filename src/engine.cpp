#include "engine.h"
#include <fstream>
using namespace std;

Engine::Engine() : keys() {
    this->initWindow();
    this->initShaders();
    this->initShapes();
}

Engine::~Engine() {}

unsigned int Engine::initWindow(bool debug) {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, false);

    window = glfwCreateWindow(width, height, "engine", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    // OpenGL configuration
    glViewport(0, 0, width, height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glfwSwapInterval(1);

    return 0;
}

void Engine::initShaders() {
    // load shader manager
    shaderManager = make_unique<ShaderManager>();

    // Load shader into shader manager and retrieve it
    shapeShader = this->shaderManager->loadShader("../res/shaders/shape.vert", "../res/shaders/shape.frag",  nullptr, "shape");

    // Set uniforms
    textShader.setVector2f("vertex", vec4(100, 100, .5, .5));
    shapeShader.use();
    shapeShader.setMatrix4("projection", this->PROJECTION);
}

void Engine::initShapes() {
//    vec2 p1 = project(vec4(0.5, 0.5, 0.4, 1));
//    vec2 p2 = project(vec4(-0.5, -0.5, 0.4, 1));
//    vec2 p3 = project(vec4(-0.5, 0.5, 0.4, 1));
//    vec2 p4 = project(vec4(0.5, -0.5, 0.4, 1));
//
//    vec2 p5 = project(vec4(0.5, 0.5, 0.25, 1));
//    vec2 p6 = project(vec4(-0.5, -0.5, 0.25, 1));
//    vec2 p7 = project(vec4(-0.5, 0.5, 0.25, 1));
//    vec2 p8 = project(vec4(0.5, -0.5, 0.25, 1));

   triangles.push_back(make_unique<Triangle>(vec4(0.5, 0.5, 0.4, 1), vec4(-0.5, -0.5, 0.4, 1), vec4(-0.5, 0.5, 0.4, 1), perspective, rotation, screenWidth, screenHeight));
   triangles.push_back(make_unique<Triangle>(vec4(0.5, 0.5, 0.4, 1), vec4(-0.5, -0.5, 0.4, 1), vec4(0.5, -0.5, 0.4, 1), perspective, rotation, screenWidth, screenHeight));


}

void Engine::processInput() {
    glfwPollEvents();

    // Set keys to true if pressed, false if released
    for (int key = 0; key < 1024; ++key) {
        if (glfwGetKey(window, key) == GLFW_PRESS)
            keys[key] = true;
        else if (glfwGetKey(window, key) == GLFW_RELEASE)
            keys[key] = false;
    }

    // Close window if escape key is pressed
    if (keys[GLFW_KEY_ESCAPE])
        glfwSetWindowShouldClose(window, true);

    // Mouse position saved to check for collisions
    glfwGetCursorPos(window, &MouseX, &MouseY);

    // If mouse overlaps with spawn button, change color
    // Mouse position is inverted because the origin of the window is in the top left corner
    MouseY = height - MouseY; // Invert y-axis of mouse position
    bool mousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    // Save mousePressed for next frame
    mousePressedLastFrame = mousePressed;
}

void Engine::update() {
    // Calculate delta time
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

void Engine::render() {
    squares.clear();
    for (auto &row: pixelsAs2dArray) {
        for (auto &element: row) {
            element = color(1, 0, 0);
        }
    }
    int x;
    int y;
    for(auto &triangle: triangles){
        triangle->Render();
        while(!triangle->drawHere.empty()){
            x = triangle->drawHere.back().x;
            y = triangle->drawHere.back().y;
            pixelsAs2dArray[x][y] = color(1,1,1);
            triangle->drawHere.pop_back();
        }
        triangle->drawHere.clear();//just in case
    }

    int xCoord = 0, yCoord = height-SIDE_LENGTH;
    for(auto &row: pixelsAs2dArray){
        for(auto &element: row){
            squares.push_back(make_unique<Rect>(shapeShader, vec2(xCoord + SIDE_LENGTH/2, yCoord + SIDE_LENGTH/2), vec2(SIDE_LENGTH, SIDE_LENGTH), element));
            xCoord += SIDE_LENGTH;
        }
        xCoord=0;
        yCoord-=SIDE_LENGTH;
    }


    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color
    glClear(GL_COLOR_BUFFER_BIT);

    // Set shader to use for all shapes
    shapeShader.use();

    for (unique_ptr<Shape> &square : squares) {
        square->setUniforms();
        square->draw();
    }

    glfwSwapBuffers(window);
}

void Engine::readFromFile(std::string filepath) {
    ifstream ins(filepath);
    if (!ins) {
        cout << "Error opening file" << endl;
    }
    ins >> noskipws;
    int xCoord = 0, yCoord = height-SIDE_LENGTH;
    char letter;
    bool draw;
    color c;
    while (ins >> letter) {
        draw = true;
        switch(letter) {
            case 'r': c = color(1, 0, 0); break;
            case 'g': c = color(0, 1, 0); break;
            case 'b': c = color(0, 0, 1); break;
            case 'y': c = color(1, 1, 0); break;
            case 'm': c = color(1, 0, 1); break;
            case 'c': c = color(0, 1, 1); break;
            case ' ': c = color(0, 0, 0); break;
            default: // newline
                draw = false;
                xCoord = 0;
                yCoord -= SIDE_LENGTH;
        }
        if (draw) {
            squares.push_back(make_unique<Rect>(shapeShader, vec2(xCoord + SIDE_LENGTH/2, yCoord + SIDE_LENGTH/2), vec2(SIDE_LENGTH, SIDE_LENGTH), c));
            xCoord += SIDE_LENGTH;
        }
    }
    ins.close();
}



bool Engine::shouldClose() {
    return glfwWindowShouldClose(window);
}

GLenum Engine::glCheckError_(const char *file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        string error;
        switch (errorCode) {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        cout << error << " | " << file << " (" << line << ")" << endl;
    }
    return errorCode;
}