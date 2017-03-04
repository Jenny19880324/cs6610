#include <stdio.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <string>
#include <cassert>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define CY_GL_DONT_CHECK_CONTEXT
#include <cyCodeBase/cyGL.h>
#include <cyCodeBase/cyTriMesh.h>
#include <cyCodeBase/cyMatrix.h>
#include <lodepng.h>

#define PI 3.141592653589

using namespace std;
using namespace cy;

////////////////////////////////////////////////////////////////////////////////
//GLOBAL VARIABLE
////////////////////////////////////////////////////////////////////////////////
TriMesh *g_mesh;
TriMesh *g_cube;
GLFWwindow *g_window;
GLSLProgram *g_teapot_program;
GLSLProgram *g_plane_program;
GLSLProgram *g_depth_program;
GLRenderDepth<GL_TEXTURE_2D> *g_render_depth;
int g_screen_width = 800;
int g_screen_height = 600;
float g_plane_width = 10.0;
float g_plane_height = 10.0;
//Bounding Box
Point3f g_minV(FLT_MAX, FLT_MAX, FLT_MAX);
Point3f g_maxV(FLT_MIN, FLT_MIN, FLT_MIN);
Point3f g_centerV(0, 0, 0);
Point3f g = Point3f(0.0, 0.0, -1.0); // gaze direction
Point3f e = Point3f(0.0, 0.0, 0.0); // eye position
Point3f t = Point3f(0.0, 1.0, 0.0); // view up direction
Point3f g_light_position(20, 30, 20);

float g_teapot_dist;
float g_plane_dist = 6.0;
//mouse operation
Point2f g_teapot_dist_record_coord;
Point2f g_teapot_angle_record_coord;
Point2f g_plane_dist_record_coord;
Point2f g_plane_angle_record_coord;
Point2f g_light_record_coord;

bool g_teapot_dist_record = false;
bool g_teapot_angle_record = false;
bool g_plane_dist_record = false;
bool g_plane_angle_record = false;
bool g_light_record = false;

Matrix4<float> g_teapot_view_matrix;
Matrix4<float> g_teapot_model_matrix;
Matrix4<float> g_teapot_projection_matrix;
Matrix4<float> g_teapot_model_view_matrix;
Matrix4<float> g_teapot_normal_transform_matrix;
Matrix4<float> g_teapot_model_view_projection_matrix;
Matrix4<float> g_teapot_mouse_rotation_matrix;
Matrix4<float> g_plane_view_matrix;
Matrix4<float> g_plane_model_matrix;
Matrix4<float> g_plane_projection_matrix;
Matrix4<float> g_plane_model_view_matrix;
Matrix4<float> g_plane_normal_transform_matrix;
Matrix4<float> g_plane_model_view_projection_matrix;
Matrix4<float> g_plane_mouse_rotation_matrix;
Matrix4<float> g_light_rotation_matrix;
Matrix4<float> g_light_view_matrix;
Matrix4<float> g_light_model_matrix;
Matrix4<float> g_light_projection_matrix;
Matrix4<float> g_light_model_view_matrix;
Matrix4<float> g_light_model_view_projection_matrix;
Matrix4<float> g_light_mouse_rotation_matrix;


GLint teapot_vertex_position_location;
GLint teapot_vertex_normal_location;
GLint plane_vertex_normal_location;
GLint plane_vertex_position_location;
GLuint g_teapot_VAO;
GLuint g_plane_VAO;
GLuint g_depth_VAO;

TriMesh::Mtl g_mtl;
////////////////////////////////////////////////////////////////////////////////
void setLightModelViewProjectionMatrix() {
	e = g_light_position;
	g = g_centerV-e;
	Point3f w = -g / g.Length();
	Point3f u = t.Cross(w) / (t.Cross(w)).Length();
	Point3f v = w.Cross(u);

	g_light_view_matrix.Set(u, v, w, e);
	g_light_view_matrix.Invert();
	g_light_model_matrix.SetIdentity();
	g_light_model_matrix = g_light_mouse_rotation_matrix * g_light_model_matrix;
	float aspect = (float)g_screen_width / (float)g_screen_height;
	g_light_projection_matrix.SetIdentity();
	g_light_projection_matrix.SetPerspective(PI / 3, aspect, 20, -20);
	g_light_model_view_matrix = g_light_view_matrix * g_light_model_matrix;
	g_light_model_view_projection_matrix = g_light_projection_matrix * g_light_view_matrix * g_light_model_matrix;

	glUseProgram(g_depth_program->GetID());

	g_depth_program->SetUniformMatrix4(0, g_light_model_view_projection_matrix.data);
}

void setTeapotModelViewProjectionMatrix() {
	//transformation
	e = Point3f(-10.0, 20.0, -60.0);
	g = g_centerV - e;
	Point3f w = -g / g.Length();
	Point3f u = t.Cross(w) / (t.Cross(w)).Length();
	Point3f v = w.Cross(u);

	g_teapot_view_matrix.Set(u, v, w, e);
	g_teapot_view_matrix.Invert();
	g_teapot_model_matrix.SetIdentity();
	g_teapot_model_matrix = g_teapot_mouse_rotation_matrix * g_teapot_model_matrix;
	float aspect = (float)g_screen_width / (float)g_screen_height;
	g_teapot_projection_matrix.SetPerspective(PI / 3, aspect, 20, -20);
	g_teapot_model_view_matrix = g_teapot_view_matrix * g_teapot_model_matrix;
	Matrix4<float> temp = g_teapot_model_view_matrix.GetInverse();
	g_teapot_normal_transform_matrix = temp.GetTranspose();
	g_teapot_model_view_projection_matrix = g_teapot_projection_matrix * g_teapot_view_matrix * g_teapot_model_matrix;

	Matrix4<float> rz;
	rz.SetRotation(Point3f(0, 0, 1), g_light_record_coord.y);
	Matrix4<float> rx;
	rx.SetRotation(Point3f(0, 1, 0), g_light_record_coord.x);
	g_light_rotation_matrix = rx * rz;

	glUseProgram(g_teapot_program->GetID());
	g_teapot_program->SetUniformMatrix4(0, g_teapot_model_view_projection_matrix.data);
	g_teapot_program->SetUniformMatrix4(1, g_teapot_normal_transform_matrix.data);
	g_teapot_program->SetUniformMatrix4(2, g_teapot_model_view_matrix.data);
	g_teapot_program->SetUniformMatrix4(3, g_light_model_view_projection_matrix.data);

}

void setPlaneModelViewProjectionMatrix(){
    e = Point3f(-10.0, 20.0, -60);
    g = g_centerV - e;
    Point3f w = -g / g.Length();
    Point3f u = t.Cross(w) / (t.Cross(w)).Length();
    Point3f v = w.Cross(u);

    g_plane_view_matrix.Set(u, v, w, e);
    g_plane_view_matrix.Invert();
    g_plane_model_matrix.SetIdentity();
    g_plane_model_matrix = g_plane_mouse_rotation_matrix * g_plane_model_matrix;
    float aspect = (float)g_screen_width/(float)g_screen_height;
    g_plane_projection_matrix.SetIdentity();
    g_plane_projection_matrix.SetPerspective(PI/3, aspect, 20, -20);
    g_plane_model_view_matrix = g_plane_view_matrix * g_plane_model_matrix;
	Matrix4<float> temp = g_plane_model_view_matrix.GetInverse();
	g_plane_normal_transform_matrix = temp.GetTranspose();
    g_plane_model_view_projection_matrix = g_plane_projection_matrix * g_plane_view_matrix * g_plane_model_matrix;

    glUseProgram(g_plane_program->GetID());
    g_plane_program->SetUniformMatrix4(0, g_plane_model_view_projection_matrix.data);
    g_plane_program->SetUniformMatrix4(1, g_plane_normal_transform_matrix.data);
    g_plane_program->SetUniformMatrix4(2, g_plane_model_view_matrix.data);
	g_plane_program->SetUniformMatrix4(3, g_light_model_view_matrix.data);
}

void setupPlaneBuffers(){
    Point3f *vertex_data = (Point3f *)malloc(sizeof(Point3f) * 6);
	Point3f *normal_data = (Point3f *)malloc(sizeof(Point3f) * 6);
    vertex_data[0] = Point3f(-20.0, 0.0, -20.0);
    vertex_data[1] = Point3f(-20.0, 0.0,  20.0);
    vertex_data[2] = Point3f( 20.0, 0.0, -20.0);
    vertex_data[3] = Point3f(-20.0, 0.0,  20.0);
    vertex_data[4] = Point3f( 20.0, 0.0,  20.0);
    vertex_data[5] = Point3f( 20.0, 0.0, -20.0);

	normal_data[0] = Point3f(0, 1, 0);
	normal_data[1] = Point3f(0, 1, 0);
	normal_data[2] = Point3f(0, 1, 0);
	normal_data[3] = Point3f(0, 1, 0);
	normal_data[4] = Point3f(0, 1, 0);
	normal_data[5] = Point3f(0, 1, 0);

    //Generate a vertex buffer
    GLuint vertex_position_buffer;
    glGenBuffers(1, &vertex_position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_position_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Point3f) * 6, vertex_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(plane_vertex_position_location);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_position_buffer);
    glVertexAttribPointer(plane_vertex_position_location, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//Generate a normal buffer
	GLuint vertex_normal_buffer;
	glGenBuffers(1, &vertex_normal_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_normal_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Point3f) * 6, vertex_data, GL_STATIC_DRAW);
	glEnableVertexAttribArray(plane_vertex_normal_location);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_normal_buffer);
	glVertexAttribPointer(plane_vertex_normal_location, 3, GL_FLOAT, GL_FALSE, 0, 0);

    free(vertex_data);
	free(normal_data);
}

void setupTeapotBuffers() {
	Point3f *vertex_data = (Point3f *)malloc(sizeof(Point3f) * g_mesh->NF() * 3);
	Point3f *normal_data = (Point3f *)malloc(sizeof(Point3f) * g_mesh->NF() * 3);
	for (int i = 0; i < g_mesh->NF(); i++) {
		vertex_data[i * 3 + 0] = g_mesh->V(g_mesh->F(i).v[0]);
		vertex_data[i * 3 + 1] = g_mesh->V(g_mesh->F(i).v[1]);
		vertex_data[i * 3 + 2] = g_mesh->V(g_mesh->F(i).v[2]);

		normal_data[i * 3 + 0] = g_mesh->VN(g_mesh->FN(i).v[0]);
		normal_data[i * 3 + 1] = g_mesh->VN(g_mesh->FN(i).v[1]);
		normal_data[i * 3 + 2] = g_mesh->VN(g_mesh->FN(i).v[2]);
	}

	//Generate a vertex buffer
	//and set its data using the vertices read from .obj file
	GLuint vertex_position_buffer;
	glGenBuffers(1, &vertex_position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_position_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Point3f) * g_mesh->NF() * 3, vertex_data, GL_STATIC_DRAW);
	glEnableVertexAttribArray(teapot_vertex_position_location);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_position_buffer);
	glVertexAttribPointer(teapot_vertex_position_location, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//Generate a normal buffer
	GLuint vertex_normal_buffer;
	glGenBuffers(1, &vertex_normal_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_normal_buffer);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Point3f) * g_mesh->NF() * 3, vertex_data, GL_STATIC_DRAW);
	glEnableVertexAttribArray(teapot_vertex_normal_location);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_normal_buffer);
	glVertexAttribPointer(teapot_vertex_normal_location, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

void setupDepthBuffers() {
	Point3f *vertex_data = (Point3f *)malloc(sizeof(Point3f) * g_mesh->NF() * 3);
	for (int i = 0; i < g_mesh->NF(); i++) {
		vertex_data[i * 3 + 0] = g_mesh->V(g_mesh->F(i).v[0]);
		vertex_data[i * 3 + 1] = g_mesh->V(g_mesh->F(i).v[1]);
		vertex_data[i * 3 + 2] = g_mesh->V(g_mesh->F(i).v[2]);
	}

	//Generate a vertex buffer
	//and set its data using the vertices read from .obj file
	GLuint vertex_position_buffer;
	glGenBuffers(1, &vertex_position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_position_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Point3f) * g_mesh->NF() * 3, vertex_data, GL_STATIC_DRAW);
	glEnableVertexAttribArray(teapot_vertex_position_location);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_position_buffer);
	glVertexAttribPointer(teapot_vertex_position_location, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

void onDisplay(){
    //render the texture
    g_render_depth->Bind();
	setLightModelViewProjectionMatrix();
    glUseProgram(g_depth_program->GetID());
    glBindVertexArray(g_depth_VAO);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnableVertexAttribArray(teapot_vertex_position_location);
    glDrawArrays(GL_TRIANGLES, 0, g_mesh->NF() * 3);
    g_render_depth->Unbind();

    //render the scene
    glUseProgram(g_plane_program->GetID());
    glBindVertexArray(g_plane_VAO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnableVertexAttribArray(plane_vertex_position_location);
    glEnableVertexAttribArray(plane_vertex_normal_location);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glUseProgram(g_teapot_program->GetID());
    glBindVertexArray(g_teapot_VAO);
    glEnableVertexAttribArray(teapot_vertex_position_location);
    glEnableVertexAttribArray(teapot_vertex_normal_location);
    glDrawArrays(GL_TRIANGLES, 0, g_mesh->NF() * 3);

    glfwSwapBuffers(g_window);
}



void cursor_position_callback(GLFWwindow *window, double xpos, double ypos){
    if(g_plane_angle_record){
        Matrix4<float> rz;
        rz.SetRotation(Point3f(1, 0, 0), (ypos - g_plane_angle_record_coord.y) * 3.14 / 180);
        Matrix4<float> rx;
        rx.SetRotation(Point3f(0, 1, 0), (xpos - g_plane_angle_record_coord.x) * 3.14 / 180);
        g_plane_mouse_rotation_matrix = rx * rz;
    }
    if(g_plane_dist_record){
        g_plane_dist = ypos - g_plane_dist_record_coord.y;
    }
    if(g_teapot_angle_record){
        Matrix4<float> rz;
        rz.SetRotation(Point3f(1, 0, 0), (ypos - g_teapot_angle_record_coord.y) * 3.14 / 180);
        Matrix4<float> rx;
        rx.SetRotation(Point3f(0, 1, 0), (xpos - g_teapot_angle_record_coord.x) * 3.14 / 180);
        g_teapot_mouse_rotation_matrix = rx * rz;
   }
    if(g_teapot_dist_record){
        g_teapot_dist = ypos - g_teapot_dist_record_coord.y;
    }
    if(g_light_record){
        g_light_record_coord.x = xpos;
        g_light_record_coord.y = ypos;
    }
    setPlaneModelViewProjectionMatrix();
    setTeapotModelViewProjectionMatrix();
}

void mouse_button_callback(GLFWwindow *window,int button, int action, int mods){
    double xpos, ypos;
    glfwGetCursorPos(g_window, &xpos, &ypos);
    if(button == GLFW_MOUSE_BUTTON_RIGHT){
        if(mods == GLFW_MOD_ALT){
            if(action == GLFW_PRESS){
                if(!g_plane_angle_record){
                    g_plane_angle_record_coord = Point2f(xpos, ypos);
                    g_plane_angle_record = true;
                }
            }
            else{
                g_plane_angle_record = false;
            }
        }
        else{
            if(action == GLFW_PRESS){
                if(!g_teapot_angle_record){
                    g_teapot_angle_record_coord = Point2f(xpos, ypos);
                    g_teapot_angle_record = true;
                }
            }
            else{
                g_teapot_angle_record = false;
            }
        }
    }
    else if(button == GLFW_MOUSE_BUTTON_LEFT){
        if(action == GLFW_PRESS){
            if(mods == GLFW_MOD_CONTROL){
                if(!g_light_record){
                    g_light_record_coord = Point2f(xpos, ypos);
                    g_light_record = true;
                }
            }
            else if(mods == GLFW_MOD_ALT){
                if(!g_plane_dist_record){
                    g_plane_dist_record_coord = Point2f(xpos, ypos);
                    g_plane_dist_record = true;
                }
            }
            else{
                if(!g_teapot_dist_record){
                    g_teapot_dist_record_coord = Point2f(xpos, ypos);
                    g_teapot_dist_record = true;
                }
           }
        }
        else{
            g_light_record = false;
            g_plane_dist_record = false;
            g_teapot_dist_record = false;
        }
    }
    setPlaneModelViewProjectionMatrix();
    setTeapotModelViewProjectionMatrix();
}

static void error_callback(int error, const char* description){
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
  if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
      glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

inline bool bindDepthTexture() {
	//depth textures
	g_render_depth->Initialize(true);
	g_render_depth->Resize(g_screen_width, g_screen_height);
	if (!g_render_depth->IsComplete()) {
		printf("buffer not complete\n"); return false;
	}
	g_render_depth->BindTexture(0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	g_render_depth->BuildTextureMipmaps();
	g_render_depth->SetTextureMaxAnisotropy();
	g_render_depth->SetTextureFilteringMode(GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST);
	return true;
}

inline void renderDepth() {
	//Generate and bind a vertex array object
	glGenVertexArrays(1, &g_depth_VAO);
	glBindVertexArray(g_depth_VAO);

	g_depth_program = new GLSLProgram();
	g_depth_program->BuildFiles("../glsl/depth.vert", "../glsl/depth.frag");
	g_depth_program->RegisterUniform(0, "modelViewProjection");

	g_depth_program->Bind();
	teapot_vertex_position_location = glGetAttribLocation(g_depth_program->GetID(), "pos");
	setLightModelViewProjectionMatrix();
	setupDepthBuffers();
}

inline void renderPlane(){
    glGenVertexArrays(1, &g_plane_VAO);
    glBindVertexArray(g_plane_VAO);

    g_plane_program = new GLSLProgram();
    g_plane_program->BuildFiles("../glsl/plane.vert", "../glsl/plane.frag");
    g_plane_program->RegisterUniform(0, "modelViewProjection");
	g_plane_program->RegisterUniform(1, "normalTransform");
	g_plane_program->RegisterUniform(2, "modelView");
	g_plane_program->RegisterUniform(3, "lightModelViewProjection");
    g_plane_program->Bind();
    plane_vertex_position_location = glGetAttribLocation(g_plane_program->GetID(), "pos");
    plane_vertex_normal_location = glGetAttribLocation(g_plane_program->GetID(), "inputNormal");

    setPlaneModelViewProjectionMatrix();
    setupPlaneBuffers();

	//depth texture
	g_render_depth->BindTexture(0);
	GLint texLoc = glGetUniformLocation(g_plane_program->GetID(), "map_Shadow");
	glUniform1i(texLoc, g_render_depth->GetTextureID());
}

inline void renderTeapot() {
	//Generate and bind a vertex array object
	glGenVertexArrays(1, &g_teapot_VAO);
	glBindVertexArray(g_teapot_VAO);

	g_teapot_program = new GLSLProgram();
	g_teapot_program->BuildFiles("../glsl/teapot.vert", "../glsl/teapot.frag");
	g_teapot_program->RegisterUniform(0, "modelViewProjection");
	g_teapot_program->RegisterUniform(1, "normalTransform");
	g_teapot_program->RegisterUniform(2, "modelView");
	g_teapot_program->RegisterUniform(3, "lightModelViewProjection");

	g_teapot_program->Bind();
	teapot_vertex_position_location = glGetAttribLocation(g_teapot_program->GetID(), "pos");
	teapot_vertex_normal_location = glGetAttribLocation(g_teapot_program->GetID(), "inputNormal");

	setTeapotModelViewProjectionMatrix();
	setupTeapotBuffers();
	
	//depth texture
	g_render_depth->BindTexture(1);
	GLint texLoc = glGetUniformLocation(g_teapot_program->GetID(), "map_Shadow");
	glUniform1i(texLoc, g_render_depth->GetTextureID());
}

int main(int argc, char *argv[]){
    //reading and parsing of the obj file
    string obj_filename;
    if(argc == 2){
        obj_filename = string(argv[1]);
    }
    else{
        cerr << "please input the obj filename" << endl;
        return 0;
    }

    g_mesh = new TriMesh();
    if(!g_mesh->LoadFromFileObj(obj_filename.c_str(), true)){
        cerr << "failure of loading the obj file" << endl;
    }
	g_mesh->ComputeNormals(false);

    //center the object
    for(unsigned i = 0; i < g_mesh->NV(); i++){
        if(g_mesh->V(i).x < g_minV.x) g_minV.x = g_mesh->V(i).x;
        if(g_mesh->V(i).y < g_minV.y) g_minV.y = g_mesh->V(i).y;
        if(g_mesh->V(i).z < g_minV.z) g_minV.z = g_mesh->V(i).z;
        if(g_mesh->V(i).x > g_maxV.x) g_maxV.x = g_mesh->V(i).x;
        if(g_mesh->V(i).y > g_maxV.y) g_maxV.y = g_mesh->V(i).y;
        if(g_mesh->V(i).z > g_maxV.z) g_maxV.z = g_mesh->V(i).z;
    }
    g_centerV = Point3f((g_minV + g_maxV) / 2);
    g_teapot_dist = (g_maxV - g_minV).z * 3;
    g_plane_mouse_rotation_matrix.SetIdentity();
    g_teapot_mouse_rotation_matrix.SetIdentity();
	g_light_mouse_rotation_matrix.SetIdentity();

    //GLFW window
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()){
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    g_window = glfwCreateWindow(g_screen_width, g_screen_height, "Project 7 of cs6610", NULL, NULL);
    if (!g_window){
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwSetKeyCallback(g_window, key_callback);
    glfwSetCursorPosCallback(g_window, cursor_position_callback);
    glfwSetMouseButtonCallback(g_window, mouse_button_callback);
    glfwMakeContextCurrent(g_window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);

	g_render_depth = new GLRenderDepth2D();
	bindDepthTexture();
	renderDepth();
	renderPlane();
	renderTeapot();

    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(g_window)){
        glViewport(0, 0, g_screen_width, g_screen_height);
        onDisplay();
        glfwPollEvents();
    }
    glfwDestroyWindow(g_window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
