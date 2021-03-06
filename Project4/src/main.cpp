#include <stdio.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <string>
#include <cassert>

//#ifdef __APPLE__
//    #include <OpenGL/glu.h>
//#elif _WIN32_
//    #include <GL/glu.h>
//#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cyCodeBase/cyTriMesh.h>
#include <cyCodeBase/cyGL.h>
#include <cyCodeBase/cyMatrix.h>
#include <lodepng.h>

#define PI 3.141592653589

using namespace std;
using namespace cy;

////////////////////////////////////////////////////////////////////////////////
//GLOBAL VARIABLE
////////////////////////////////////////////////////////////////////////////////
TriMesh *g_mesh;
GLFWwindow *g_window;
GLSLProgram *g_program;
int g_screen_width = 800;
int g_screen_height = 600;
//Bounding Box
Point3f g_minV(FLT_MAX, FLT_MAX, FLT_MAX);
Point3f g_maxV(FLT_MIN, FLT_MIN, FLT_MIN);
Point3f g_centerV(0, 0, 0);
Point3f g = Point3f(0.0, 0.0, -1.0); // gaze direction
Point3f e = Point3f(0.0, 0.0, 0.0); // eye position
Point3f t = Point3f(0.0, 1.0, 0.0); // view up direction
Point3f g_light_position = Point3f(100, 0, 0);

float dist;
//mouse operation
Point2f g_dist_record_coord;
Point2f g_angle_record_coord;
Point2f g_light_record_coord;

bool g_dist_record = false;
bool g_angle_record = false;
bool g_light_record = false;

Matrix4<float> g_view_matrix;
Matrix4<float> g_model_matrix;
Matrix4<float> g_projection_matrix;
Matrix4<float> g_model_view_matrix;
Matrix4<float> g_normal_transform_matrix;
Matrix4<float> g_model_view_projection_matrix;
Matrix4<float> g_light_rotation_matrix;
Matrix4<float> g_mouse_rotation_matrix;

GLint vertex_position_location;
GLint vertex_normal_location;
GLint vertex_texcoord_location;

TriMesh::Mtl g_mtl;
////////////////////////////////////////////////////////////////////////////////
void setModelViewProjectionMatrix(){
    //transformation
    e = Point3f(0.0, 20.0, dist);
    Point3f w = -g / g.Length();
    Point3f u = t.Cross(w) / (t.Cross(w)).Length();
    Point3f v = w.Cross(u);

    g_view_matrix.Set(u, v, w, e);
    g_view_matrix.Invert();
    g_model_matrix.SetRotationX(-PI/2);
    g_model_matrix = g_mouse_rotation_matrix * g_model_matrix;
    float aspect = (float)g_screen_width/(float)g_screen_height;
    g_projection_matrix.SetPerspective(PI/3, aspect, 20, -20);
    g_model_view_matrix = g_view_matrix * g_model_matrix;
    Matrix4<float> temp = g_model_view_matrix.GetInverse();
    g_normal_transform_matrix = temp.GetTranspose();
    g_model_view_projection_matrix = g_projection_matrix * g_view_matrix * g_model_matrix;

    Matrix4<float> rz;
    rz.SetRotation(Point3f(0, 0, 1), g_light_record_coord.y);
    Matrix4<float> rx;
    rx.SetRotation(Point3f(0, 1, 0), g_light_record_coord.x);
    g_light_rotation_matrix = rx * rz;

    g_program->SetUniform(0, g_light_position);
    g_program->SetUniformMatrix4(1, g_model_view_projection_matrix.data);
    g_program->SetUniformMatrix4(2, g_normal_transform_matrix.data);
    g_program->SetUniformMatrix4(3, g_model_view_matrix.data);
}

void setupBuffers(){
    Point3f *vertex_data = (Point3f *)malloc(sizeof(Point3f) * g_mesh->NF() * 3);
    Point3f *normal_data = (Point3f *)malloc(sizeof(Point3f) * g_mesh->NF() * 3);
    Point3f *texcoord_data = (Point3f *)malloc(sizeof(Point3f) * g_mesh->NF() * 3);
    for(int i = 0; i < g_mesh->NF(); i++){
        vertex_data[i * 3 + 0] = g_mesh->V(g_mesh->F(i).v[0]);
        vertex_data[i * 3 + 1] = g_mesh->V(g_mesh->F(i).v[1]);
        vertex_data[i * 3 + 2] = g_mesh->V(g_mesh->F(i).v[2]);

        normal_data[i * 3 + 0] = g_mesh->VN(g_mesh->FN(i).v[0]);
        normal_data[i * 3 + 1] = g_mesh->VN(g_mesh->FN(i).v[1]);
        normal_data[i * 3 + 2] = g_mesh->VN(g_mesh->FN(i).v[2]);

        texcoord_data[i * 3 + 0] = g_mesh->VT(g_mesh->FT(i).v[0]);
        texcoord_data[i * 3 + 1] = g_mesh->VT(g_mesh->FT(i).v[1]);
        texcoord_data[i * 3 + 2] = g_mesh->VT(g_mesh->FT(i).v[2]);
    }

    //Generate a vertex buffer
    //and set its data using the vertices read from .obj file
    GLuint vertex_position_buffer;
    glGenBuffers(1, &vertex_position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_position_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Point3f) * g_mesh->NF() * 3, vertex_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(vertex_position_location);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_position_buffer);
    glVertexAttribPointer(vertex_position_location, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //Generate a normal buffer
    GLuint vertex_normal_buffer;
    glGenBuffers(1, &vertex_normal_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_normal_buffer);

    glBufferData(GL_ARRAY_BUFFER, sizeof(Point3f) * g_mesh->NF() * 3, vertex_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(vertex_normal_location);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_normal_buffer);
    glVertexAttribPointer(vertex_normal_location, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //Generate a texture coordinate buffer
    GLuint vertex_texcoord_buffer;
    glGenBuffers(1, &vertex_texcoord_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_texcoord_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Point2f) * g_mesh->NF() * 3, texcoord_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(vertex_texcoord_location);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_texcoord_buffer);
    glVertexAttribPointer(vertex_texcoord_location, 3, GL_FLOAT, GL_FALSE, 0, 0);
}


void onDisplay(){
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f ,1.0f);
    glDrawArrays(GL_TRIANGLES, 0, g_mesh->NF() * 3);
    //glDrawElements(GL_TRIANGLES, g_mesh->NF() * 3, GL_UNSIGNED_INT, (void*)0);
    glfwSwapBuffers(g_window);
}

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos){
    if(g_angle_record){
        Matrix4<float> rz;
        rz.SetRotation(Point3f(1, 0, 0), ypos - g_angle_record_coord.y);
        Matrix4<float> rx;
        rx.SetRotation(Point3f(0, 1, 0), xpos - g_angle_record_coord.x);
        g_mouse_rotation_matrix = rx * rz;
   }
    if(g_dist_record){
        dist = ypos - g_dist_record_coord.y;
    }
    if(g_light_record){
        g_light_record_coord.x = xpos;
        g_light_record_coord.y = ypos;
    }
    setModelViewProjectionMatrix();
}

void mouse_button_callback(GLFWwindow *window,int button, int action, int mods){
    double xpos, ypos;
    glfwGetCursorPos(g_window, &xpos, &ypos);
    if(button == GLFW_MOUSE_BUTTON_RIGHT){
        if(action == GLFW_PRESS){
            if(!g_angle_record){
                g_angle_record_coord = Point2f(xpos, ypos);
                g_angle_record = true;
            }
        }
        else{
           g_angle_record = false;
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
            else{
                if(!g_dist_record){
                    g_dist_record_coord = Point2f(xpos, ypos);
                    g_dist_record = true;
                }
           }
        }
        else{
            g_light_record = false;
            g_dist_record = false;
        }
    }
    setModelViewProjectionMatrix();
}

static void error_callback(int error, const char* description){
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
  if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
      glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
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
    dist = (g_maxV - g_minV).z * 3;
    e = Point3f(0.0, 0.0, dist);
    g = g_centerV - e;
    g_mouse_rotation_matrix.SetIdentity();

    //GLFW window
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()){
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    g_window = glfwCreateWindow(g_screen_width, g_screen_height, "Project 4 of cs6610", NULL, NULL);
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

    //Generate and bind a vertex array object
    GLboolean isVAO;
    GLuint m_VAO;
    GLuint vertexBuffer;
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);


    //glsl shader
    g_program = new GLSLProgram();
    g_program->BuildFiles("../glsl/vert.txt", "../glsl/frag.txt");
    g_program->RegisterUniform(0, "lightPosition");
    g_program->RegisterUniform(1, "modelViewProjection");
    g_program->RegisterUniform(2, "normalTransform");
    g_program->RegisterUniform(3, "modelView");
    g_program->RegisterUniform(4, "Ns");
    g_program->RegisterUniform(5, "Ka");
    g_program->RegisterUniform(6, "Kd");
    g_program->RegisterUniform(7, "Ks");

    g_program->Bind();
    vertex_position_location = glGetAttribLocation(g_program->GetID(), "pos");
    vertex_normal_location = glGetAttribLocation(g_program->GetID(), "inputNormal");
    vertex_texcoord_location = glGetAttribLocation(g_program->GetID(), "inputTexCoord");

    Point3f p_Ka; p_Ka.Set(g_mesh->M(0).Ka);
    Point3f p_Kd; p_Kd.Set(g_mesh->M(0).Kd);
    Point3f p_Ks; p_Ks.Set(g_mesh->M(0).Ks);

    g_program->SetUniform(4, g_mesh->M(0).Ns);
    g_program->SetUniform(5, p_Ka);
    g_program->SetUniform(6, p_Kd);
    g_program->SetUniform(7, p_Ks);
    setModelViewProjectionMatrix();
    setupBuffers();

    //textures
    const char *path = "../teapot/brick.png";
    string map_Ka_filename = "../teapot/" + string(g_mesh->M(0).map_Ka);
    string map_Kd_filename = "../teapot/" + string(g_mesh->M(0).map_Kd);
    string map_Ks_filename = "../teapot/" + string(g_mesh->M(0).map_Ks);

    unsigned width, height;
    vector<unsigned char> map_Ka_image;
    vector<unsigned char> map_Kd_image;
    vector<unsigned char> map_Ks_image;
    unsigned error = lodepng::decode(map_Ka_image, width, height,map_Ka_filename.c_str());
    if(error){
        cout << "decoder error " << error << ": " << lodepng_error_text(error) <<endl;
    }
    error = lodepng::decode(map_Kd_image, width, height, map_Kd_filename.c_str());
    if(error){
        cout << "decoder error " << error << ": " << lodepng_error_text(error) << endl;
    }

    error = lodepng::decode(map_Ks_image, width, height, map_Ks_filename.c_str());
    if(error){
        cout << "decoder error " << error << ": " << lodepng_error_text(error) << endl;
    }

    assert(map_Ka_image.size() == 512 * 512 * 4);
    assert(map_Kd_image.size() == 512 * 512 * 4);
    assert(map_Ks_image.size() == 512 * 512 * 4);
    GLubyte *map_Ka_data = (GLubyte *)malloc(sizeof(GLubyte) * 512 * 512 * 4);
    GLubyte *map_Kd_data = (GLubyte *)malloc(sizeof(GLubyte) * 512 * 512 * 4);
    GLubyte *map_Ks_data = (GLubyte *)malloc(sizeof(GLubyte) * 512 * 512 * 4);

    for(int i = 0; i < map_Ka_image.size(); i++){
        map_Ka_data[i] = map_Ka_image[i];
        map_Kd_data[i] = map_Kd_image[i];
        map_Ks_data[i] = map_Ks_image[i];
    }

    GLTexture<GL_TEXTURE_2D> map_Ka;
    GLTexture<GL_TEXTURE_2D> map_Kd;
    GLTexture<GL_TEXTURE_2D> map_Ks;
    map_Ka.Initialize();
    map_Kd.Initialize();
    map_Ks.Initialize();
    map_Ka.SetImage(map_Ka_data, GL_RGBA, GL_RGBA, width, height);
    map_Kd.SetImage(map_Kd_data, GL_RGBA, GL_RGBA, width, height);
    map_Ks.SetImage(map_Ks_data, GL_RGBA, GL_RGBA, width, height);

    map_Ka.BuildMipmaps();
    map_Kd.BuildMipmaps();
    map_Ks.BuildMipmaps();

    map_Ka.SetWrappingMode(GL_REPEAT);
    map_Kd.SetWrappingMode(GL_REPEAT);
    map_Ks.SetWrappingMode(GL_REPEAT);

    map_Ka.SetFilteringMode(GL_NEAREST, GL_LINEAR);
    map_Kd.SetFilteringMode(GL_NEAREST, GL_LINEAR);
    map_Ks.SetFilteringMode(GL_NEAREST, GL_LINEAR);

    //free(map_Ka_data);
    //free(map_Kd_data);
    //free(map_Ks_data);
    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(g_window)){
        glViewport(0, 0, g_screen_width, g_screen_height);
        glUseProgram(g_program->GetID());
        onDisplay();
        glfwPollEvents();
    }
    glfwDestroyWindow(g_window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
