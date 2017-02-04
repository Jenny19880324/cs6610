#include <stdio.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <string>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#else
#include <windows.h>
#include <GL/glew.h>
#include <GL/gl.h>
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

#include "cyTriMesh.h"
#include "cyGL.h"
#include "cyMatrix.h"

#define PI 3.141592653589

using namespace std;
using namespace cy;

////////////////////////////////////////////////////////////////////////////////
//GLOBAL VARIABLE
////////////////////////////////////////////////////////////////////////////////
TriMesh *g_mesh;
GLSLProgram *g_program;
unsigned g_screen_width = 800;
unsigned g_screen_height = 600;
//Bounding Box
Point3f g_minV(FLT_MAX, FLT_MAX, FLT_MAX);
Point3f g_maxV(FLT_MIN, FLT_MIN, FLT_MIN);
Point3f g_centerV(0, 0, 0);
Point3f g; // gaze direction
Point3f t = Point3f(0.0, 1.0, 0.0); // view up direction
float dist;
//mouse operation
Point2f g_dist_record_coord;
Point2f g_angle_record_coord;
bool g_dist_record = false;
bool g_angle_record = false;
Matrix4<float> g_view_matrix;
Matrix4<float> g_projection_matrix;
Matrix4<float> g_model_view_projection_matrix;

////////////////////////////////////////////////////////////////////////////////
void setModelViewProjectionMatrix(){
    //transformation
    Point3f w = -g / g.Length();
    Point3f u = t.Cross(w) / (t.Cross(w)).Length();
    Point3f v = w.Cross(u);
    Point3f e(0.0, 0.0, -dist);

    g_view_matrix.Set(u, v, w, e);
    float aspect = (float)g_screen_width/(float)g_screen_height;
    g_projection_matrix.SetPerspective(PI/3, aspect, 20, -20);
    g_model_view_projection_matrix = g_projection_matrix * g_view_matrix;

    g_program->SetUniformMatrix4(0, g_model_view_projection_matrix.data);
}

void setupBuffers(){
    //Generate a vertex buffer
    //and set its data using the vertices read from .obj file
    GLuint vertex_position_buffer;
    glGenBuffers(1, &vertex_position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_position_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Point3f) * g_mesh->NV(), &g_mesh->V(0), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_position_buffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //Generate buffers for the triangle vertices of an .obj file
    GLuint vertex_index_buffer;
    glGenBuffers(1, &vertex_index_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_index_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cy::TriMesh::TriFace) * g_mesh->NF(), &g_mesh->F(0),GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_index_buffer);
    glVertexAttribPointer(1, 3, GL_UNSIGNED_INT, GL_FALSE, 0, 0);
}

void onDisplay(){
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f ,1.0f);

    glDrawElements(GL_TRIANGLES, g_mesh->NF(), GL_UNSIGNED_INT, &g_mesh->F(0));

    glutSwapBuffers();
}

void onMotion(int x, int y){
    if(g_angle_record){
        g.x = (x - g_angle_record_coord.x) * 0.1;
        g.y = (y - g_angle_record_coord.y) * 0.1;
    }
    if(g_dist_record){
        dist = y - g_dist_record_coord.y;
    }
    setModelViewProjectionMatrix();
    glutPostRedisplay();
}

void onMouse(int button, int state, int x, int y){
    if(button == GLUT_LEFT_BUTTON){
        if(state == GLUT_DOWN){
            if(!g_angle_record){
                g_angle_record_coord = Point2f(x, y);
                g_angle_record = true;
            }
        }
        else{
           g_angle_record = false;
        }
    }
    else if(button == GLUT_RIGHT_BUTTON){
        if(state == GLUT_DOWN){
            if(!g_dist_record){
                g_dist_record_coord = Point2f(x,y);
                g_dist_record = true;
            }
        }
        else{
            g_dist_record = false;
        }
    }
    setModelViewProjectionMatrix();
    glutPostRedisplay();
}

void onKeyboard(unsigned char key, int x, int y){
    if(key == 27){
        exit(0);
    }
}


void onSpecial(int key, int x, int y){
    if(key == GLUT_KEY_F6){
        cout << "recompile the GLSL shaders" << endl;
        g_program->BuildFiles("../glsl/vert.txt", "../glsl/frag.txt");
    }
}

void onIdle(){
    glutPostRedisplay();
}

void timeout(int value){
    glutPostRedisplay();
	glutTimerFunc(1000, timeout, 0);
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
    dist = (g_maxV - g_minV).z * 2;
    g = -g_centerV;

    srand(time(NULL));
    glutInit(&argc, argv);
    glutInitWindowSize(g_screen_width, g_screen_height);
    glutCreateWindow("Project 3 of CS6610");

    //Generate and bind a vertex array object
   GLboolean isVAO;
   const GLubyte *strExt;
   const GLubyte *strVersion;
   float myGLVersion;
   strExt = glGetString (GL_EXTENSIONS);
   strVersion = glGetString(GL_VERSION);
   sscanf((char *)strVersion, "%f", &myGLVersion);
   cout << "myGLVersion = " << myGLVersion << endl;

#ifdef __APPLE__
   isVAO = gluCheckExtension((const GLubyte*)"GL_APPLE_vertex_array_object", strExt);
    if(!isVAO){
        cerr << "isVAO false" << endl;
        return 1;
    }
#endif

#ifdef _WIN64
	//generate and bind a vertex array object
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		/* Problem: glewInit failed, something is seriously wrong */
		cerr << "Error: " << glewGetErrorString(err) << endl;
		return 1;
	}
	cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << endl;
#endif

    GLuint m_VAO;
    GLuint vertexBuffer;
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    glBindVertexArray(0);


    //glsl shader
    g_program = new GLSLProgram();
    glBindAttribLocation(g_program->GetID(), 0, "pos");
    g_program->BuildFiles("../glsl/vert.txt", "../glsl/frag.txt");
    g_program->RegisterUniform(0, "modelViewProjection");
    g_program->Bind();
    setModelViewProjectionMatrix();
    setupBuffers();

    glutDisplayFunc(onDisplay);
    glutKeyboardFunc(onKeyboard);
    glutSpecialFunc(onSpecial);
    glutMouseFunc(onMouse);
    glutMotionFunc(onMotion);
	//glutTimerFunc(1000, timeout, 0);
    glutIdleFunc(onIdle);
    glutMainLoop();
}
