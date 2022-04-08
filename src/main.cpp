#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
unsigned int loadTexture(char const * path);
unsigned int loadCubemap(vector<std::string> faces);
// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool lamp = false;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float heightScale = 0.5;

struct ProgramState {
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    float cons = 1.0f;
    float lin = 0.09;
    float quad = 0.032;
    float spotLightRadius = 0.0f;
    ProgramState()
            : camera(glm::vec3(0.0f, 0.0f, -3.0f)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << ImGuiEnabled << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n'
        << cons << '\n'
        << lin << '\n'
        << quad << '\n'
        << spotLightRadius << '\n';
}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
            in >> ImGuiEnabled
            >> camera.Position.x
            >> camera.Position.y
            >> camera.Position.z
            >> camera.Front.x
            >> camera.Front.y
            >> camera.Front.z
            >> cons
            >> lin
            >> quad
            >> spotLightRadius;
    }
}

ProgramState *programState;

void DrawImGui(ProgramState *programState);

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    programState->camera.Position = glm::vec3(-1.5f, 0.3f, 20.0f);
    programState->camera.Front = glm::vec3(0.0,0.0,0.0);
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader modelShader("resources/shaders/model.vs", "resources/shaders/model.fs");
    Shader grassShader("resources/shaders/grass.vs", "resources/shaders/grass.fs");
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    Shader windowShader("resources/shaders/window.vs", "resources/shaders/window.fs");
    Shader lightShader("resources/shaders/light.vs", "resources/shaders/light.fs");
    Shader roomShader("resources/shaders/room.vs", "resources/shaders/room.fs");
    // load models
    // -----------
    Model roomModel("resources/objects/room/room.obj");
    Model tableModel("resources/objects/table/table.obj");
    Model closetModel("resources/objects/closet/uploads_files_2750161_Wardrobes.obj");
    Model appleModel("resources/objects/apple/apple.obj");
    Model notebookModel("resources/objects/notebook/Lowpoly_Notebook_2.obj");
    Model coffeeModel("resources/objects/coffee/coffee_cup_obj.obj");
    Model chairModel("resources/objects/chair/uploads_files_2164682_Office_chair_type_03.obj");
    Model lightModel("resources/objects/light/light.obj");

    roomModel.SetShaderTextureNamePrefix("material.");
    tableModel.SetShaderTextureNamePrefix("material.");
    closetModel.SetShaderTextureNamePrefix("material.");
    appleModel.SetShaderTextureNamePrefix("material.");
    notebookModel.SetShaderTextureNamePrefix("material.");
    coffeeModel.SetShaderTextureNamePrefix("material.");
    chairModel.SetShaderTextureNamePrefix("material.");
    lightModel.SetShaderTextureNamePrefix("material.");

    glm::vec3 pos1(-20.0f,  20.0f, 0.0f);
    glm::vec3 pos2(-20.0f, -20.0f, 0.0f);
    glm::vec3 pos3( 20.0f, -20.0f, 0.0f);
    glm::vec3 pos4( 20.0f,  20.0f, 0.0f);
    // texture coordinates
    glm::vec2 uv1(0.0f, 40.0f);
    glm::vec2 uv2(0.0f, 0.0f);
    glm::vec2 uv3(40.0f, 0.0f);
    glm::vec2 uv4(40.0f, 40.0f);
    // normal vector
    glm::vec3 nm(0.0f, 0.0f, 1.0f);

    // calculate tangent/bitangent vectors of both triangles
    glm::vec3 tangent1, bitangent1;
    glm::vec3 tangent2, bitangent2;
    // triangle 1
    // ----------
    glm::vec3 edge1 = pos2 - pos1;
    glm::vec3 edge2 = pos3 - pos1;
    glm::vec2 deltaUV1 = uv2 - uv1;
    glm::vec2 deltaUV2 = uv3 - uv1;

    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

    bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

    // triangle 2
    // ----------
    edge1 = pos3 - pos1;
    edge2 = pos4 - pos1;
    deltaUV1 = uv3 - uv1;
    deltaUV2 = uv4 - uv1;

    f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);


    bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

    float grassVertices[] = {
            // positions            // normal         // texcoords  // tangent                          // bitangent
            pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
            pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
            pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

            pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
            pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
            pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
    };

    float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };

    float windowVertices[] = {
            // positions          // texture Coords
            1.2f, 0.0f,  0.0f,  1.0f, 0.0f,
            0.0f, 0.0f,  0.0f,  0.0f, 0.0f,
            0.0f, 0.0f, -2.0f,  0.0f, 1.0f,
            1.2f, 0.0f,  0.0f,  1.0f, 0.0f,
            0.0f, 0.0f, -2.0f,  0.0f, 1.0f,
            1.2f, 0.0f, -2.0f,  1.0f, 1.0f,
    };

    unsigned int grassVAO, grassVBO;
    glGenVertexArrays(1, &grassVAO);
    glGenBuffers(1, &grassVBO);
    glBindVertexArray(grassVAO);
    glBindBuffer(GL_ARRAY_BUFFER, grassVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(grassVertices), &grassVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    unsigned int windowVAO, windowVBO;
    glGenVertexArrays(1, &windowVAO);
    glGenBuffers(1, &windowVBO);
    glBindVertexArray(windowVAO);
    glBindBuffer(GL_ARRAY_BUFFER, windowVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(windowVertices), &windowVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    unsigned int grassDiffuse = loadTexture(FileSystem::getPath("resources/textures/Green-Grass-Ground-Texture-DIFFUSE.jpg").c_str());
    unsigned int grassSpecular = loadTexture(FileSystem::getPath("resources/textures/Green-Grass-Ground-Texture-SPECULAR.jpg").c_str());
    unsigned int grassNormal = loadTexture(FileSystem::getPath("resources/textures/Green-Grass-Ground-Texture-NORMAL.jpg").c_str());
    unsigned int grassHeight = loadTexture(FileSystem::getPath("resources/textures/Green-Grass-Ground-Texture-DISP.jpg").c_str());

    unsigned int windowTexture = loadTexture(FileSystem::getPath("resources/textures/window.png").c_str());

    vector<std::string> faces
    {
            FileSystem::getPath("resources/textures/skybox/px.png"),
            FileSystem::getPath("resources/textures/skybox/nx.png"),
            FileSystem::getPath("resources/textures/skybox/py.png"),
            FileSystem::getPath("resources/textures/skybox/ny.png"),
            FileSystem::getPath("resources/textures/skybox/pz.png"),
            FileSystem::getPath("resources/textures/skybox/nz.png")
    };

    unsigned int cubemapTexture = loadCubemap(faces);

    grassShader.use();
    grassShader.setInt("material.diffuse", 0);
    grassShader.setInt("material.specular", 1);
    grassShader.setInt("material.normal", 2);
    grassShader.setInt("material.depth", 3);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    windowShader.use();
    windowShader.setInt("texture1", 0);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        grassShader.use();
        grassShader.setVec3("viewPos", programState->camera.Position);
        grassShader.setFloat("material.shininess", 64.0f);

        grassShader.setVec3("dirLdirection", 0.91f, 0.33f, -0.23f);
        grassShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        grassShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        grassShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

        grassShader.setVec3("spotLposition", programState->camera.Position);
        grassShader.setVec3("spotLdirection", programState->camera.Front);
        grassShader.setVec3("spotLight.ambient", 0.05f, 0.05f, 0.05f);
        grassShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        grassShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        grassShader.setFloat("spotLight.constant", programState->cons);
        grassShader.setFloat("spotLight.linear", programState->lin);
        grassShader.setFloat("spotLight.quadratic", programState->quad);
        grassShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(programState->spotLightRadius)));
        grassShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(programState->spotLightRadius + 2.5f)));
        grassShader.setBool("spotLight.lamp", lamp);

        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f,-0.0005f,0.0f));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0,0.0,0.0));
        grassShader.setMat4("projection", projection);
        grassShader.setMat4("view", view);
        grassShader.setMat4("model", model);
        grassShader.setFloat("heightScale", heightScale);

        glBindVertexArray(grassVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grassDiffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, grassSpecular);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, grassNormal);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, grassHeight);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        roomShader.use();
        roomShader.setVec3("viewPos", programState->camera.Position);
        roomShader.setFloat("material.shininess", 64.0f);

        roomShader.setVec3("dirLdirection", 0.91f, 0.33f, -0.23f);
        roomShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        roomShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        roomShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

        roomShader.setVec3("spotLposition", programState->camera.Position);
        roomShader.setVec3("spotLdirection", programState->camera.Front);
        roomShader.setVec3("spotLight.ambient", 0.05f, 0.05f, 0.05f);
        roomShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        roomShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        roomShader.setFloat("spotLight.constant", programState->cons);
        roomShader.setFloat("spotLight.linear", programState->lin);
        roomShader.setFloat("spotLight.quadratic", programState->quad);
        roomShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(programState->spotLightRadius)));
        roomShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(programState->spotLightRadius + 2.5f)));
        roomShader.setBool("spotLight.lamp", lamp);

        roomShader.setVec3("pointLposition1", 2.0f,3.8f,0.0f);
        roomShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        roomShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        roomShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        roomShader.setFloat("pointLights[0].constant", programState->cons);
        roomShader.setFloat("pointLights[0].linear", programState->lin);
        roomShader.setFloat("pointLights[0].quadratic", programState->quad);

        roomShader.setVec3("pointLposition2", -2.0f,3.8f,4.0f);
        roomShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
        roomShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
        roomShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
        roomShader.setFloat("pointLights[1].constant", programState->cons);
        roomShader.setFloat("pointLights[1].linear", programState->lin);
        roomShader.setFloat("pointLights[1].quadratic", programState->quad);

        roomShader.setMat4("projection", projection);
        roomShader.setMat4("view", view);
        roomShader.setFloat("heightScale", heightScale);

        model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        roomShader.setMat4("model", model);
        roomModel.Draw(roomShader);

        modelShader.use();
        modelShader.setVec3("viewPos", programState->camera.Position);
        modelShader.setFloat("material.shininess", 64.0f);

        modelShader.setVec3("dirLdirection", 0.91f, 0.33f, -0.23f);
        modelShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        modelShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        modelShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

        modelShader.setVec3("spotLposition", programState->camera.Position);
        modelShader.setVec3("spotLdirection", programState->camera.Front);
        modelShader.setVec3("spotLight.ambient", 0.05f, 0.05f, 0.05f);
        modelShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        modelShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        modelShader.setFloat("spotLight.constant", programState->cons);
        modelShader.setFloat("spotLight.linear", programState->lin);
        modelShader.setFloat("spotLight.quadratic", programState->quad);
        modelShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(programState->spotLightRadius)));
        modelShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(programState->spotLightRadius + 2.5f)));
        modelShader.setBool("spotLight.lamp", lamp);

        modelShader.setVec3("pointLposition1", 2.0f,3.8f,0.0f);
        modelShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        modelShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        modelShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        modelShader.setFloat("pointLights[0].constant", programState->cons);
        modelShader.setFloat("pointLights[0].linear", programState->lin);
        modelShader.setFloat("pointLights[0].quadratic", programState->quad);

        modelShader.setVec3("pointLposition2", -2.0f,3.8f,4.0f);
        modelShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
        modelShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
        modelShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
        modelShader.setFloat("pointLights[1].constant", programState->cons);
        modelShader.setFloat("pointLights[1].linear", programState->lin);
        modelShader.setFloat("pointLights[1].quadratic", programState->quad);

        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(3.0,0.33,0.0));
        model = glm::scale(model, glm::vec3(1.5));
        modelShader.setMat4("model", model);
        tableModel.Draw(modelShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.8,1.24,0.7));
        model = glm::scale(model, glm::vec3(0.3));
        modelShader.setMat4("model", model);
        appleModel.Draw(modelShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(3.0,1.27,0.0));
        model = glm::scale(model, glm::vec3(0.3));
        modelShader.setMat4("model", model);
        notebookModel.Draw(modelShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(3.0,1.27,0.0));
        model = glm::scale(model, glm::vec3(0.3));
        modelShader.setMat4("model", model);
        notebookModel.Draw(modelShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.0,0.0,-1.2));
        model = glm::scale(model, glm::vec3(1.6));
        modelShader.setMat4("model", model);
        closetModel.Draw(modelShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.5,1.24,0.8));
        model = glm::rotate(model, glm::radians(-120.0f), glm::vec3(0.0,1.0,0.0));
        model = glm::scale(model, glm::vec3(0.4));
        modelShader.setMat4("model", model);
        coffeeModel.Draw(modelShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.0,0.0,0.0));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0,1.0,0.0));
        model = glm::scale(model, glm::vec3(0.7));
        modelShader.setMat4("model", model);
        chairModel.Draw(modelShader);


        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        lightShader.use();
        lightShader.setMat4("projection", projection);
        lightShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0,3.84,0.0));
        model = glm::scale(model, glm::vec3(0.3));
        lightShader.setMat4("model", model);
        lightModel.Draw(lightShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.0,3.84,4.0));
        model = glm::scale(model, glm::vec3(0.3));
        lightShader.setMat4("model", model);
        lightModel.Draw(lightShader);
        glDisable(GL_CULL_FACE);



        windowShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(windowVAO);
        glBindTexture(GL_TEXTURE_2D, windowTexture);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.4,1.0,2.0));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        windowShader.setMat4("projection", projection);
        windowShader.setMat4("view", view);
        windowShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        projection = glm::perspective(glm::radians(programState->camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view = glm::mat4(glm::mat3(programState->camera.GetViewMatrix())); // remove translation from the view matrix
        model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0,1.0,0.0));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0,0.0,1.0));

        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        skyboxShader.setMat4("model", model);

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);


        if (programState->ImGuiEnabled)
            DrawImGui(programState);


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(ProgramState *programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        static float f = 0.0f;
        ImGui::Begin("Hello window");
        ImGui::Text("Hello text");

        ImGui::DragFloat("constant", &programState->cons, 0.05, 0.0, 1.0);
        ImGui::DragFloat("linear", &programState->lin, 0.05, 0.0, 1.0);
        ImGui::DragFloat("quadratic", &programState->quad, 0.05, 0.0, 1.0);
        ImGui::DragFloat("lamp radius", &programState->spotLightRadius, 0.05, 0.0, 30.0);
        ImGui::End();
    }

    {
        ImGui::Begin("Camera info");
        const Camera& c = programState->camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
    if (key == GLFW_KEY_L && action == GLFW_PRESS){
        lamp = !lamp;
    }
}

unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}