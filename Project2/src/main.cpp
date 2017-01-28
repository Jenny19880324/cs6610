#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <string>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#define __gl_h_
#include <OpenGL/gl3ext.h>
#else
#include <windows.h>
#include <GL/gl.h>
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

#include "cyTriMesh.h"

using namespace std;
using namespace cy;

////////////////////////////////////////////////////////////////////////////////
//GLOBAL VARIABLE
////////////////////////////////////////////////////////////////////////////////
TriMesh *g_mesh;

////////////////////////////////////////////////////////////////////////////////
void onDisplay(){
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor((double)rand()/RAND_MAX,(double)rand()/RAND_MAX,(double)rand()/RAND_MAX,1.0f);

     //Generate a vertex buffer and set its data using the vertices read from .obj file
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Point3f) * g_mesh->NV(), &g_mesh->V(0), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_POINTS, 0, g_mesh->NV());

    glutSwapBuffers();
}

void onKeyboard(unsigned char key, int x, int y){
    if(key == 27){
        exit(0);
    }
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

    srand(time(NULL));
    glutInit(&argc, argv);
    //generate and bind a vertex array object
//    GLenum err = glewInit();
//    if(GLEW_OK != err){
        /* Problem: glewInit failed, something is seriously wrong */
//        cerr << "Error: " << glewGetErrorString(err) << endl;
//        return 1;
//    }
//    cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << endl;

    unsigned screen_width = 800;
    unsigned screen_height = 600;
    glutInitWindowSize(screen_width, screen_height);

   glutCreateWindow("Project 1 of CS6610");

    //Generate and bind a vertex array object
   GLboolean isVAO;
   const GLubyte *strExt;
   strExt = glGetString (GL_EXTENSIONS);
   isVAO = gluCheckExtension((const GLubyte*)"GL_APPLE_vertex_array_object", strExt);
    if(!isVAO){
        cerr << "isVAO false" << endl;
        return 1;
    }
    GLuint m_VAO;
    GLuint vertexBuffer;
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    glBindVertexArray(0);

    glutDisplayFunc(onDisplay);
    glutKeyboardFunc(onKeyboard);
	glutTimerFunc(1000, timeout, 0);
    glutMainLoop();
}
