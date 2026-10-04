/******************************************************************************
 *                                                                            *
 *  Copyright (c) 2025 Ojaswa Sharma. All rights reserved.                    *
 *                                                                            *
 *  Author: Ojaswa Sharma                                                     *
 *  E-mail: ojaswa@iiitd.ac.in                                                *
 *                                                                            *
 *  This code is provided solely for the purpose of the CSE 333/533 course    *
 *  at IIIT Delhi. Unauthorized reproduction, distribution, or disclosure     *
 *  of this code, in whole or in part, without the prior written consent of   *
 *  the author is strictly prohibited.                                        *
 *                                                                            *
 *  This code is provided "as is", without warranty of any kind, express      *
 *  or implied, including but not limited to the warranties of                *
 *  merchantability, fitness for a particular purpose, and noninfringement.   *
 *                                                                            *
 ******************************************************************************/ 

//Assignment 02: Trandformations, viewing and projection

#include "utils.h"

#define SPIRAL_R 25.0
#define SPIRAL_H 25.0
#define SPIRAL_K 8

int width = 640, height=640;
bool showSpiral = true;
float spiralTheta = 0.0; // spiral theta parameter for camera animation
float spiralDeltaTheta = 0.25*M_PI/180.0; // Increment by 0.25 degree in every update
float updateInterval = 0.01667f; // Update animation every 16.67 ms = 60 fps
glm::vec3 camPos; // Camera position

bool isPaused = false; // animation pause/play
bool isOrthographic = false; // orthographic/perspective projection



void createAxes(unsigned int &, unsigned int &);
void spiral(const float &theta, float &x, float &y, float &z);
unsigned int createSpiral(unsigned int &, unsigned int &);
void createCube(unsigned int &, unsigned int &);
void setupModelTransformation(unsigned int &);
void setupViewTransformation(unsigned int &);
void setupProjectionTransformation(unsigned int &, int, int);

int main(int, char**)
{
    // Setup window
    GLFWwindow *window = setupWindow(width, height);
    ImGuiIO& io = ImGui::GetIO(); // Create IO object

    ImVec4 clearColor = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);

    unsigned int shaderProgram = createProgram("./shaders/vshader.vs", "./shaders/fshader.fs");
    glUseProgram(shaderProgram);

    unsigned int axes_VAO, spiral_VAO, cube_VAO;

    setupModelTransformation(shaderProgram);
    setupViewTransformation(shaderProgram);
    setupProjectionTransformation(shaderProgram, width , height);

    createAxes(shaderProgram, axes_VAO);
    unsigned int n_spiralVertices = createSpiral(shaderProgram, spiral_VAO);
    createCube(shaderProgram, cube_VAO);

	static float timeAccumulator = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glUseProgram(shaderProgram);
        if (!io.WantCaptureKeyboard && ImGui::IsKeyPressed(GLFW_KEY_S)) 
            showSpiral = !showSpiral;

        // Update spiral theta variable based on time
        float dt = ImGui::GetIO().DeltaTime;
        timeAccumulator += dt;
        if (timeAccumulator >= updateInterval) {
            timeAccumulator -= updateInterval;
            if (!isPaused) {
                spiralTheta += spiralDeltaTheta;
                if(spiralTheta >= 2.0*M_PI) spiralTheta -= 2.0*M_PI;
            }
        }
        
		// ImGui UI
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Information", NULL, ImGuiWindowFlags_AlwaysAutoResize);                          
            ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Text("Spiral is %s. Press S key to toggle", showSpiral?"displayed":"not displayed");
            ImGui::Text("Camera position: (%.2f, %.2f, %.2f)", camPos.x, camPos.y, camPos.z);
            ImGui::Text("Projection: %s. Press P/O keys to toggle", isOrthographic?"Orthographic":"Perspective");
            ImGui::Text("Animation is %s. Press Space key to toggle", isPaused?"paused":"running");
            ImGui::Text("Spiral theta: %.2f degrees. Use arrow keys to increase/decrease.", spiralTheta*180.0/M_PI);
            ImGui::End();
        }

        if (!io.WantCaptureKeyboard && ImGui::IsKeyPressed(GLFW_KEY_SPACE)) isPaused = !isPaused;
        if (!io.WantCaptureKeyboard && ImGui::IsKeyPressed(GLFW_KEY_P)) isOrthographic = false;
        if (!io.WantCaptureKeyboard && ImGui::IsKeyPressed(GLFW_KEY_O)) isOrthographic = true;
        if (!io.WantCaptureKeyboard && ImGui::IsKeyPressed(GLFW_KEY_LEFT) && isPaused) spiralTheta -= spiralDeltaTheta;
        if (!io.WantCaptureKeyboard && ImGui::IsKeyPressed(GLFW_KEY_RIGHT) && isPaused) spiralTheta += spiralDeltaTheta;
        if (!io.WantCaptureKeyboard && ImGui::IsKeyPressed(GLFW_KEY_UP) && isPaused) spiralTheta += spiralDeltaTheta;
        if (!io.WantCaptureKeyboard && ImGui::IsKeyPressed(GLFW_KEY_DOWN) && isPaused) spiralTheta -= spiralDeltaTheta;
        if(spiralTheta >= 2.0*M_PI) spiralTheta -= 2.0*M_PI;
        if(spiralTheta < 0) spiralTheta += 2.0*M_PI;


        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(axes_VAO); 
        glDrawArrays(GL_LINES, 0, 6);

        if(showSpiral) {
            glBindVertexArray(spiral_VAO);
            glDrawArrays(GL_LINE_STRIP, 0, n_spiralVertices);
        }

        glBindVertexArray(cube_VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6*2*3);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        setupModelTransformation(shaderProgram);
        setupViewTransformation(shaderProgram);
        setupProjectionTransformation(shaderProgram, width , height);

        glfwSwapBuffers(window);

    }

    // Cleanup
    cleanup(window);

    return 0;
}

void createAxes(unsigned int &program, unsigned int &axes_VAO)
{
    glUseProgram(program);

    //Bind shader variables
    int vVertex_attrib = glGetAttribLocation(program, "vVertex");
    if(vVertex_attrib == -1) {
        fprintf(stderr, "Could not bind location: vVertex\n");
        exit(0);
    }
    int vColor_attrib = glGetAttribLocation(program, "vColor");
    if(vColor_attrib == -1) {
        fprintf(stderr, "Could not bind location: vColor\n");
        exit(0);
    }

    // Axes data
    GLfloat axes_vertices[] = { 100, 0, 0, -100, 0, 0, 0, 100, 0, 0, -100, 0, 0, 0, 100, 0, 0, -100};
    GLfloat axes_colors[] = {1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1}; // (Red - X, Green - Y, Blue - Z)
    int nVertices = 6; //(3 axes) * (2 vertices each)
    
    // Generate VAO object
    glGenVertexArrays(1, &axes_VAO);
    glBindVertexArray(axes_VAO);

    // Create VBOs for the VAO
    GLuint vertex_VBO, color_VBO;
    glGenBuffers(1, &vertex_VBO);
    glGenBuffers(1, &color_VBO);

    // Bind and set vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vertex_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axes_vertices), axes_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(vVertex_attrib);
    glVertexAttribPointer(vVertex_attrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Bind and set color data
    glBindBuffer(GL_ARRAY_BUFFER, color_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axes_colors), axes_colors, GL_STATIC_DRAW);
    glEnableVertexAttribArray(vColor_attrib);
    glVertexAttribPointer(vColor_attrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); // Unbind the VAO to disable changes outside this function.
}

// Evaluate the periodic spiral at a given theta (in radians) parameter value
// Spiral Eqn.:
// x(\theta) &= \sqrt{R^2 + 4h^2\cos(\theta)}\, \cos{(k\theta)}\\
// y(\theta) &= h\cos{(\theta)}
// z(\theta) &= \sqrt{R^2 + 4h^2\cos(\theta)}\, \sin{(k\theta)}\\
// where \theta\in[0, 2\pi]
void spiral(const float &theta, float &x, float &y, float &z)
{
		y = SPIRAL_H*cosf(theta);
		float rad = sqrtf(SPIRAL_R*SPIRAL_R + 4.0*y*y);
		x = rad*cosf(SPIRAL_K*theta);
		z = rad*sinf(SPIRAL_K*theta);
}

unsigned int createSpiral(unsigned int &program, unsigned int &spiral_VAO)
{
    glUseProgram(program);

    //Bind shader variables
    int vVertex_attrib = glGetAttribLocation(program, "vVertex");
    if(vVertex_attrib == -1) {
        fprintf(stderr, "Could not bind location: vVertex\n");
        exit(0);
    }
    int vColor_attrib = glGetAttribLocation(program, "vColor");
    if(vColor_attrib == -1) {
        fprintf(stderr, "Could not bind location: vColor\n");
        exit(0);
    }

    unsigned int nVertices = SPIRAL_K*180;

    GLfloat *spiral_vertices = new GLfloat[nVertices*3];
		float theta, x, y, z;
    for(unsigned int i=0; i<nVertices; i++) {
				theta = (float)i/(nVertices - 1)*2.0*M_PI;
				spiral(theta, x, y, z);
        spiral_vertices[i*3] = x;
        spiral_vertices[i*3 + 1] = y;
        spiral_vertices[i*3 + 2] = z;
    }

    GLfloat *spiral_colors = new GLfloat[nVertices*3]; // Add some fun color variation to our spiral
    for(int i=0; i<nVertices; i++) {
        float t = (float)i / (nVertices - 1);
        spiral_colors[i*3] = t; // Red
        spiral_colors[i*3 + 1] = (t<0.5)?(2.0*t):(2.0-2.0*t); // Green
        spiral_colors[i*3 + 2] = 1.0 - t; // Blue
    }
    
    //Generate VAO object
    glGenVertexArrays(1, &spiral_VAO);
    glBindVertexArray(spiral_VAO);

    //Create VBOs for the VAO
    GLuint vertex_VBO, color_VBO;
    glGenBuffers(1, &vertex_VBO);
    glGenBuffers(1, &color_VBO);

    // Bind and set vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vertex_VBO);
    glBufferData(GL_ARRAY_BUFFER, nVertices*3*sizeof(GLfloat), spiral_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(vVertex_attrib);
    glVertexAttribPointer(vVertex_attrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Bind and set color data
    glBindBuffer(GL_ARRAY_BUFFER, color_VBO);
    glBufferData(GL_ARRAY_BUFFER, nVertices*3*sizeof(GLfloat), spiral_colors, GL_STATIC_DRAW);
    glEnableVertexAttribArray(vColor_attrib);
    glVertexAttribPointer(vColor_attrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); // Unbind the VAO to disable changes outside this function.

    return nVertices;
}

void createCube(unsigned int &program, unsigned int &cube_VAO)
{
    glUseProgram(program);

    //Bind shader variables
    int vVertex_attrib = glGetAttribLocation(program, "vVertex");
    if(vVertex_attrib == -1) {
        fprintf(stderr, "Could not bind location: vVertex\n");
        exit(0);
    }
    int vColor_attrib = glGetAttribLocation(program, "vColor");
    if(vColor_attrib == -1) {
        fprintf(stderr, "Could not bind location: vColor\n");
        exit(0);
    }

    //Cube data
    float len = 5.0;
    GLfloat cube_vertices[] = {len, len, len, -len, len, len, -len, -len, len, len, -len, len, //Front
                   len, len, -len, -len, len, -len, -len, -len, -len, len, -len, -len}; //Back
    GLushort cube_indices[] = {0, 2, 3, 0, 1, 2, //Front
                4, 7, 6, 4, 6, 5, //Back
                5, 2, 1, 5, 6, 2, //Left
                4, 3, 7, 4, 0, 3, //Right
                1, 0, 4, 1, 4, 5, //Top
                2, 7, 3, 2, 6, 7}; //Bottom
    GLfloat cube_colors[] = {1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1}; //Unique face colors

    //Generate VAO object
    glGenVertexArrays(1, &cube_VAO);
    glBindVertexArray(cube_VAO);

    //Create VBOs for the VAO
    //Position information (data + format)
    int nVertices = 6*2*3; //(6 faces) * (2 triangles each) * (3 vertices each)
    GLfloat *expanded_vertices = new GLfloat[nVertices*3];
    for(int i=0; i<nVertices; i++) {
        expanded_vertices[i*3] = cube_vertices[cube_indices[i]*3];
        expanded_vertices[i*3 + 1] = cube_vertices[cube_indices[i]*3+1];
        expanded_vertices[i*3 + 2] = cube_vertices[cube_indices[i]*3+2];
    }
    GLuint vertex_VBO;
    glGenBuffers(1, &vertex_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_VBO);
    glBufferData(GL_ARRAY_BUFFER, nVertices*3*sizeof(GLfloat), expanded_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(vVertex_attrib);
    glVertexAttribPointer(vVertex_attrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
    delete []expanded_vertices;

    //Color - one for each face
    GLfloat *expanded_colors = new GLfloat[nVertices*3];
    for(int i=0; i<nVertices; i++) {
        int color_index = i / 6;
        expanded_colors[i*3] = cube_colors[color_index*3];
        expanded_colors[i*3+1] = cube_colors[color_index*3+1];
        expanded_colors[i*3+2] = cube_colors[color_index*3+2];
    }
    GLuint color_VBO;
    glGenBuffers(1, &color_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, color_VBO);
    glBufferData(GL_ARRAY_BUFFER, nVertices*3*sizeof(GLfloat), expanded_colors, GL_STATIC_DRAW);
    glEnableVertexAttribArray(vColor_attrib);
    glVertexAttribPointer(vColor_attrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
    delete []expanded_colors;

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); //Unbind the VAO to disable changes outside this function.
}

void setupModelTransformation(unsigned int &program)
{
    //Modelling transformations (Model -> World coordinates)
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.0));//Model coordinates are the world coordinates

    //Pass on the modelling matrix to the vertex shader
    glUseProgram(program);
    int vModel_uniform = glGetUniformLocation(program, "vModel");
    if(vModel_uniform == -1){
        fprintf(stderr, "Could not bind location: vModel\n");
        exit(0);
    }
    glUniformMatrix4fv(vModel_uniform, 1, GL_FALSE, glm::value_ptr(model));
}


void setupViewTransformation(unsigned int &program)
{
    
    // modify camera position baed on theta
    spiral(spiralTheta, camPos.x, camPos.y, camPos.z);
	//Viewing transformations (World -> Camera coordinates
    glm::mat4 view = glm::lookAt(camPos, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

    //Pass-on the viewing matrix to the vertex shader
    glUseProgram(program);
    int vView_uniform = glGetUniformLocation(program, "vView");
    if(vView_uniform == -1){
        fprintf(stderr, "Could not bind location: vView\n");
        exit(0);
    }
    glUniformMatrix4fv(vView_uniform, 1, GL_FALSE, glm::value_ptr(view));
}

void setupProjectionTransformation(unsigned int &program, int screen_width, int screen_height)
{
    //Projection transformation
    float aspect = (float)screen_width/(float)screen_height;

    glm::mat4 projection = (isOrthographic) ? glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, 0.1f, 1000.0f) : glm::perspective(45.0f, (GLfloat)screen_width/(GLfloat)screen_height, 0.1f, 1000.0f);

    //Pass on the projection matrix to the vertex shader
    glUseProgram(program);
    int vProjection_uniform = glGetUniformLocation(program, "vProjection");
    if(vProjection_uniform == -1){
        fprintf(stderr, "Could not bind location: vProjection\n");
        exit(0);
    }
    glUniformMatrix4fv(vProjection_uniform, 1, GL_FALSE, glm::value_ptr(projection));
}
