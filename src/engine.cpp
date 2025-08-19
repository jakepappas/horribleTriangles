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

void Engine::drawTriangle(vec2 p1, vec2 p2, vec2 p3){
    drawLine(p1,p2);
    drawLine(p1,p3);
    drawLine(p2,p3);
}

void Engine::fillTriangle(vec2 p1, vec2 p2, vec2 p3){

}
void Engine::colorPixel(int x, int y, color c){
    pixelsAs2dArray[x][y] = c;
}

// Bresenham's line algorithm
void Engine::drawLine(vec2 p1, vec2 p2) {
    int x0 = p1.x;
    int y0 = p1.y;
    int x1 = p2.x;
    int y1 = p2.y;

    bool steep = abs(y1 - y0) > abs(x1 - x0);

    if (steep) {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }

    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int dx = x1 - x0;
    int dy = abs(y1 - y0);
    int error = dx / 2;
    int ystep = (y0 < y1) ? 1 : -1;
    int y = y0;

    for (int x = x0; x <= x1; x++) {
        if (steep) {
            if (y >= 0 && y <= 128 &&
                x >= 0 && x <= 128) {
                colorPixel(y,x,WHITE);
            }
        } else {
            if (x >= 0 && x <= 128 &&
                y >= 0 && y <= 128) {
                colorPixel(x,y,WHITE);
            }
        }

        error -= dy;
        if (error < 0) {
            y += ystep;
            error += dx;
        }
    }
}




void Engine::rasterize() {
    const int screenWidth = 128;
    const int screenHeight = 128;

    const float far = 3.0f;
    const float near = 2.0f;
    const float fov = 90.0f;
    const float aspect = 1.0f;
    const float f = 1.0f / tan((fov * M_PI / 180.0f) / 2.0f);

    mat4 perspective = mat4(
            f/aspect, 0, 0, 0,
            0, f, 0, 0,
            0, 0, (far+near)/(near-far), (2*far*near)/(near-far),
            0, 0, -1, 0
    );

    const float degree = 10 * M_PI / 180.0f;

    mat4 rotation = mat4(
            cos(degree), 0, sin(degree), 0,
            0, 1, 0, 0,
            -sin(degree), 0, cos(degree), 0,
            0, 0, 0, 1
    );


    auto project = [&](vec4 point3d) -> vec2 {
        vec4 projected = perspective * rotation * point3d;
        vec3 ndc = vec3(projected) / projected.w;
        int x = (ndc.x + 1.0f) * 0.5f * screenWidth;
        int y = (1.0f - (ndc.y + 1.0f) * 0.5f) * screenHeight;
        return vec2(x, y);
    };

    vec2 p1 = project(vec4(0.5, 0.5, 0.4, 1));
    vec2 p2 = project(vec4(-0.5, -0.5, 0.4, 1));
    vec2 p3 = project(vec4(-0.5, 0.5, 0.4, 1));
    vec2 p4 = project(vec4(0.5, -0.5, 0.4, 1));

    vec2 p5 = project(vec4(0.5, 0.5, 0.25, 1));
    vec2 p6 = project(vec4(-0.5, -0.5, 0.25, 1));
    vec2 p7 = project(vec4(-0.5, 0.5, 0.25, 1));
    vec2 p8 = project(vec4(0.5, -0.5, 0.25, 1));

    drawTriangle(p5,p6,p7);
    drawTriangle(p5,p6,p8);

    drawTriangle(p1,p2,p3);
    drawTriangle(p1,p2,p4);

}



void Engine::initShapes() {
    // Relative file path from inside cmake-build-debug folder to the txt file
//    readFromFile("../res/art/scene.txt");

    for (auto &row: pixelsAs2dArray) {
        for (auto &element: row) {
            element = color(1, 0, 0);
        }
    }

    rasterize();
    int xCoord = 0, yCoord = height-SIDE_LENGTH;


    for(auto &row: pixelsAs2dArray){
        for(auto &element: row){
            squares.push_back(make_unique<Rect>(shapeShader, vec2(xCoord + SIDE_LENGTH/2, yCoord + SIDE_LENGTH/2), vec2(SIDE_LENGTH, SIDE_LENGTH), element));
            xCoord += SIDE_LENGTH;
        }
        xCoord=0;
        yCoord-=SIDE_LENGTH;
    }

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