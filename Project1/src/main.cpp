#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <windows.h>
#include <GL/gl.h>
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

void onDisplay(){
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor((double)rand()/RAND_MAX,(double)rand()/RAND_MAX,(double)rand()/RAND_MAX,1.0f);
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
    srand(time(NULL));
    glutInit(&argc, argv);

    unsigned screen_width = 800;
    unsigned screen_height = 600;
    glutInitWindowSize(screen_width, screen_height);
    glutCreateWindow("Project 1 of CS6610");

    glutDisplayFunc(onDisplay);
    glutKeyboardFunc(onKeyboard);
	glutTimerFunc(1000, timeout, 0);
    glutMainLoop();
}
