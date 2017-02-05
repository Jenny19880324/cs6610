#include <stdio.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cyCodeBase/cyTriMesh.h>
#include <cyCodeBase/cyGL.h>
#include <cyCodeBase/cyMatrix.h>

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
float dist;
//mouse operation
Point2f g_dist_record_coord;
Point2f g_angle_record_coord;
bool g_dist_record = false;
bool g_angle_record = false;
Matrix4<float> g_view_matrix;
Matrix4<float> g_model_matrix;
Matrix4<float> g_projection_matrix;
Matrix4<float> g_model_view_matrix;
Matrix4<float> g_normal_transform_matrix;
Matrix4<float> g_model_view_projection_matrix;

GLint vertex_position_location;
GLint vertex_normal_location;
GLint g_model_view_projection_matrix_location;
GLint g_normal_transform_matrix_location;
////////////////////////////////////////////////////////////////////////////////
void setModelViewProjectionMatrix(){
    //transformation
    e = Point3f(0.0, 0.0, dist);
    Point3f w = -g / g.Length();
    Point3f u = t.Cross(w) / (t.Cross(w)).Length();
    Point3f v = w.Cross(u);

//cout << "g = (" << g.x << ", " << g.y << ", " << g.z << ")" << endl;
    g_view_matrix.Set(u, v, w, e);
    g_view_matrix.Invert();
    g_model_matrix.SetRotationX(-PI/2);
    float aspect = (float)g_screen_width/(float)g_screen_height;
    g_projection_matrix.SetPerspective(PI/3, aspect, 20, -20);
    g_model_view_matrix = g_view_matrix * g_model_matrix;
    Matrix4<float> temp = g_model_view_matrix.GetInverse();
    g_normal_transform_matrix = temp.GetTranspose();
    //g_normal_transform_matrix.SetIdentity();

float *d = g_normal_transform_matrix.data;
cout << "g_normal_transform_matrix = " << d[0] << " " << d[4] << " " << d[8] << " " << d[12] << endl;
cout << "                            " << d[1] << " " << d[5] << " " << d[9] << " " << d[13] << endl;
cout << "                            " << d[2] << " " << d[6] << " " << d[10] << " " << d[14] << endl;
cout << "                            " << d[3] << " " << d[7] << " " << d[11] << " " << d[15] << endl;

//for(int i = 0; i < g_mesh->NV(); i++){
//    Point4f tn = g_normal_transform_matrix * g_mesh->VN(i);
//    cout << "tn = (" << tn.x << "," << tn.y << "," << tn.z << "," << tn.w << ")" << endl;
//}
    g_model_view_projection_matrix = g_projection_matrix * g_view_matrix * g_model_matrix;
    g_program->SetUniformMatrix4(0, g_model_view_projection_matrix.data);
    g_program->SetUniformMatrix4(1, g_normal_transform_matrix.data);
}

void setupBuffers(){
    //Generate a vertex buffer
    //and set its data using the vertices read from .obj file
    GLuint vertex_position_buffer;
    glGenBuffers(1, &vertex_position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_position_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Point3f) * g_mesh->NV(), &g_mesh->V(0), GL_STATIC_DRAW);
    glEnableVertexAttribArray(vertex_position_location);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_position_buffer);
    glVertexAttribPointer(vertex_position_location, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //Generate a normal buffer
    GLuint vertex_normal_buffer;
    glGenBuffers(1, &vertex_normal_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_normal_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Point3f) * g_mesh->NV(), &g_mesh->VN(0), GL_STATIC_DRAW);
    glEnableVertexAttribArray(vertex_normal_location);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_normal_buffer);
    glVertexAttribPointer(vertex_normal_location, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //Generate buffers for the triangle vertices of an .obj file
    GLuint vertex_index_buffer;
    glGenBuffers(1, &vertex_index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cy::TriMesh::TriFace) * g_mesh->NF(), &g_mesh->F(0),GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_index_buffer);
}


void onDisplay(){
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f ,1.0f);
    glDrawElements(GL_TRIANGLES, g_mesh->NF() * 3, GL_UNSIGNED_INT,(void*)0 );
    glfwSwapBuffers(g_window);
}

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos){
    if(g_angle_record){
        g.x = (xpos - g_angle_record_coord.x) * 0.1;
        g.y = (ypos - g_angle_record_coord.y) * 0.1;
//cout << "g = (" << g.x << "," << g.y << "," << g.z << ")" << endl;
   }
    if(g_dist_record){
        dist = ypos - g_dist_record_coord.y;
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
            if(!g_dist_record){
                g_dist_record_coord = Point2f(xpos, ypos);
                g_dist_record = true;
           }
        }
        else{
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
    if(!g_mesh->LoadFromFileObj(obj_filename.c_str(), false)){
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
//cout << "g = (" << g.x << "," << g.y << "," << g.z << ")" << endl;
    //GLFW window
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()){
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    g_window = glfwCreateWindow(g_screen_width, g_screen_height, "Project 3 of cs6610", NULL, NULL);
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
    g_program->RegisterUniform(0, "modelViewProjection");
    g_program->RegisterUniform(1, "normalTransform");
    g_program->Bind();
    vertex_position_location = glGetAttribLocation(g_program->GetID(), "pos");
    vertex_normal_location = glGetAttribLocation(g_program->GetID(), "normal");
    g_model_view_projection_matrix_location = glGetUniformLocation(g_program->GetID(), "modelViewProjection");
    g_normal_transform_matrix_location = glGetUniformLocation(g_program->GetID(), "normalTransform");
    setModelViewProjectionMatrix();
    setupBuffers();

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
