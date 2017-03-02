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
TriMesh *g_sphere;
GLFWwindow *g_window;
GLSLProgram *g_teapot_program;
GLSLProgram *g_plane_program;
GLSLProgram *g_cube_program;
GLSLProgram *g_sphere_program;
GLRenderBuffer2D *g_render_buffer;
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
Point3f g_light_position = Point3f(-1, 0, 0);

float g_teapot_dist;
float g_plane_dist = 6.0;
//mouse operation
Point2f g_teapot_dist_record_coord;
Point2f g_teapot_angle_record_coord;
Point2f g_plane_dist_record_coord;
Point2f g_plane_angle_record_coord;
Point2f g_cube_angle_record_coord;
Point2f g_light_record_coord;

bool g_teapot_dist_record = false;
bool g_teapot_angle_record = false;
bool g_plane_dist_record = false;
bool g_plane_angle_record = false;
bool g_cube_angle_record = false;
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
Matrix4<float> g_cube_view_matrix;
Matrix4<float> g_cube_model_matrix;
Matrix4<float> g_cube_projection_matrix;
Matrix4<float> g_cube_model_view_matrix;
Matrix4<float> g_cube_normal_transform_matrix;
Matrix4<float> g_cube_model_view_projection_matrix;
Matrix4<float> g_cube_mouse_rotation_matrix;
Matrix4<float> g_sphere_view_matrix;
Matrix4<float> g_sphere_model_matrix;
Matrix4<float> g_sphere_projection_matrix;
Matrix4<float> g_sphere_model_view_matrix;
Matrix4<float> g_sphere_normal_transform_matrix;
Matrix4<float> g_sphere_model_view_projection_matrix;
Matrix4<float> g_sphere_mouse_rotation_matrix;
Matrix4<float> g_light_rotation_matrix;

GLint teapot_vertex_position_location;
GLint teapot_vertex_normal_location;
GLint teapot_vertex_texcoord_location;
GLint plane_vertex_texcoord_location;
GLint plane_vertex_normal_location;
GLint plane_vertex_position_location;
GLint cube_vertex_position_location;
GLint sphere_vertex_position_location;
GLint sphere_vertex_normal_location;
GLint sphere_vertex_texcoord_location;
GLuint g_teapot_VAO;
GLuint g_plane_VAO;
GLuint g_cube_VAO;
GLuint g_sphere_VAO;

GLTextureCubeMap g_cubemap;

TriMesh::Mtl g_mtl;
////////////////////////////////////////////////////////////////////////////////
void setPlaneModelViewProjectionMatrix(){
    e = Point3f(0.0, 10.0, 40);
    g = Point3f(0.0, 0.0, -1.0);
    Point3f w = -g / g.Length();
    Point3f u = t.Cross(w) / (t.Cross(w)).Length();
    Point3f v = w.Cross(u);

    g_plane_view_matrix.Set(u, v, w, e);
    g_plane_view_matrix.Invert();
    g_plane_model_matrix.SetIdentity();
    g_plane_model_matrix = g_plane_mouse_rotation_matrix * g_plane_model_matrix;
    float aspect = (float)g_plane_width/(float)g_plane_height;
    g_plane_projection_matrix.SetIdentity();
    g_plane_projection_matrix.SetPerspective(PI/3, aspect, 20, -20);
    g_plane_model_view_matrix = g_plane_view_matrix * g_plane_model_matrix;
    g_plane_model_view_projection_matrix = g_plane_projection_matrix * g_plane_view_matrix * g_plane_model_matrix;

    glUseProgram(g_plane_program->GetID());

    g_plane_program->SetUniformMatrix4(0, g_plane_model_view_projection_matrix.data);
    g_plane_program->SetUniformMatrix4(1, g_plane_normal_transform_matrix.data);
    g_plane_program->SetUniformMatrix4(2, g_plane_model_view_matrix.data);
}


void setTeapotModelViewProjectionMatrix(){
    //transformation
    e = Point3f(0.0, 0.0, g_teapot_dist);
    g = g_centerV - e;
    Point3f w = -g / g.Length();
    Point3f u = t.Cross(w) / (t.Cross(w)).Length();
    Point3f v = w.Cross(u);

    g_teapot_view_matrix.Set(u, v, w, e);
    g_teapot_view_matrix.Invert();
    g_teapot_model_matrix.SetRotationX(-PI/2);
    g_teapot_model_matrix = g_teapot_mouse_rotation_matrix * g_teapot_model_matrix;
    float aspect = (float)g_screen_width/(float)g_screen_height;
    g_teapot_projection_matrix.SetPerspective(PI/3, aspect, 20, -20);
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
    g_teapot_program->SetUniform(0, g_light_position.x, g_light_position.y, g_light_position.z);
    g_teapot_program->SetUniformMatrix4(1, g_teapot_model_view_projection_matrix.data);
    g_teapot_program->SetUniformMatrix4(2, g_teapot_normal_transform_matrix.data);
    g_teapot_program->SetUniformMatrix4(3, g_teapot_model_view_matrix.data);
}

void setCubeModelViewProjectionMatrix(){
    e = Point3f(0.0, 0.0, 0.0);
    g = Point3f(0.0, 0.0, -1.0);
    Point3f w = -g/g.Length();
    Point3f u = t.Cross(w) / (t.Cross(w)).Length();
    Point3f v = w.Cross(u);

    g_cube_view_matrix.Set(u, v, w, e);
    g_cube_view_matrix.Invert();

    g_cube_model_matrix.SetIdentity();
    g_cube_model_matrix = g_cube_mouse_rotation_matrix * g_cube_model_matrix;
    float aspect = (float)g_screen_width / (float)g_screen_height;
    g_cube_projection_matrix.SetIdentity();
    g_cube_projection_matrix.SetPerspective(PI/3, aspect, 0, -11);
    g_cube_model_view_matrix = g_cube_view_matrix * g_cube_model_matrix;
    g_cube_model_view_projection_matrix = g_cube_projection_matrix * g_cube_view_matrix * g_cube_model_matrix;

    glUseProgram(g_cube_program->GetID());
    g_cube_program->SetUniformMatrix4(0, g_cube_model_view_projection_matrix.data);
}

void setSphereModelViewProjectionMatrix(){
    e = Point3f(0.0, 10.0, 40.0);
    g = Point3f(0.0, 0.0, -1.0);
    Point3f w = -g / g.Length();
    Point3f u = t.Cross(w) / (t.Cross(w)).Length();
    Point3f v = w.Cross(u);

    g_sphere_view_matrix.Set(u, v, w, e);
    g_sphere_view_matrix.Invert();

    g_sphere_model_matrix.SetIdentity();
    g_sphere_model_matrix = g_sphere_mouse_rotation_matrix * g_sphere_model_matrix;
    float aspect = (float)g_screen_width / (float)g_screen_height;
    g_sphere_projection_matrix.SetIdentity();
    g_sphere_projection_matrix.SetPerspective(PI/3, aspect, 20, -20);
    g_sphere_model_view_matrix = g_sphere_view_matrix * g_sphere_model_matrix;
    g_sphere_model_view_projection_matrix = g_sphere_projection_matrix * g_sphere_view_matrix * g_sphere_model_matrix;
    Matrix4<float> temp = g_sphere_model_view_matrix.GetInverse();
    g_sphere_normal_transform_matrix = temp.GetTranspose();

    glUseProgram(g_sphere_program->GetID());

    g_sphere_program->SetUniformMatrix4(1, g_sphere_model_view_projection_matrix.data);
    g_sphere_program->SetUniformMatrix4(2, g_sphere_normal_transform_matrix.data);
    g_sphere_program->SetUniformMatrix4(3, g_sphere_model_view_matrix.data);

}

void setupPlaneBuffers(){
    Point3f *vertex_data = (Point3f *)malloc(sizeof(Point3f) * 6);
    Point3f *normal_data = (Point3f *)malloc(sizeof(Point3f) * 6);
    Point2f *texcoord_data = (Point2f *)malloc(sizeof(Point2f) * 6);
    vertex_data[0] = Point3f(-10.0, 0.0, -10.0);
    vertex_data[1] = Point3f(-10.0, 0.0,  10.0);
    vertex_data[2] = Point3f( 10.0, 0.0, -10.0);
    vertex_data[3] = Point3f(-10.0, 0.0,  10.0);
    vertex_data[4] = Point3f( 10.0, 0.0,  10.0);
    vertex_data[5] = Point3f( 10.0, 0.0, -10.0);

    //vertex_data[0] = Point3f(-0.1, -0.1, 0.0);
    //vertex_data[1] = Point3f( 0.1, -0.1, 0.0);
    //vertex_data[2] = Point3f(-0.1,  0.1, 0.0);
    //vertex_data[3] = Point3f( 0.1, -0.1, 0.0);
    //vertex_data[4] = Point3f( 0.1,  0.1, 0.0);
    //vertex_data[5] = Point3f(-0.1,  0.1, 0.0);

    normal_data[0] = Point3f(0, 1, 0);
    normal_data[1] = Point3f(0, 1, 0);
    normal_data[2] = Point3f(0, 1, 0);
    normal_data[3] = Point3f(0, 1, 0);
    normal_data[4] = Point3f(0, 1, 0);
    normal_data[5] = Point3f(0, 1, 0);

    texcoord_data[0] = Point2f(0.0, 0.0);
    texcoord_data[1] = Point2f(0.0, 1.0);
    texcoord_data[2] = Point2f(1.0, 0.0);
    texcoord_data[3] = Point2f(0.0, 1.0);
    texcoord_data[4] = Point2f(1.0, 1.0);
    texcoord_data[5] = Point2f(1.0, 0.0);

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
    glBufferData(GL_ARRAY_BUFFER, sizeof(Point3f) * 6, normal_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(plane_vertex_normal_location);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_normal_buffer);
    glVertexAttribPointer(plane_vertex_normal_location, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //Generate a texture coordinate buffer
    GLuint vertex_texcoord_buffer;
    glGenBuffers(1, &vertex_texcoord_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_texcoord_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Point2f) * 6, texcoord_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(plane_vertex_texcoord_location);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_texcoord_buffer);
    glVertexAttribPointer(plane_vertex_texcoord_location, 2, GL_FLOAT, GL_FALSE, 0, 0);

    //free(vertex_data);
    //free(normal_data);
    //free(texcoord_data);
}

void setupTeapotBuffers(){
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
    glEnableVertexAttribArray(teapot_vertex_position_location);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_position_buffer);
    glVertexAttribPointer(teapot_vertex_position_location, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //Generate a normal buffer
    GLuint vertex_normal_buffer;
    glGenBuffers(1, &vertex_normal_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_normal_buffer);

    glBufferData(GL_ARRAY_BUFFER, sizeof(Point3f) * g_mesh->NF() * 3, normal_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(teapot_vertex_normal_location);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_normal_buffer);
    glVertexAttribPointer(teapot_vertex_normal_location, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //Generate a texture coordinate buffer
    GLuint vertex_texcoord_buffer;
    glGenBuffers(1, &vertex_texcoord_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_texcoord_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Point3f) * g_mesh->NF() * 3, texcoord_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(teapot_vertex_texcoord_location);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_texcoord_buffer);
    glVertexAttribPointer(teapot_vertex_texcoord_location, 3, GL_FLOAT, GL_FALSE, 0, 0);

    free(vertex_data);
    free(normal_data);
    free(texcoord_data);
}

void setupCubeBuffers(){
    Point3f *vertex_data = (Point3f *)malloc(sizeof(Point3f) * g_cube->NF() * 3);
    for(int i = 0; i < g_cube->NF(); i++){
        vertex_data[i * 3 + 0] = g_cube->V(g_cube->F(i).v[0]);
        vertex_data[i * 3 + 1] = g_cube->V(g_cube->F(i).v[1]);
        vertex_data[i * 3 + 2] = g_cube->V(g_cube->F(i).v[2]);
    }

    //Generate a vertex buffer
    //and set its data using the vertices read from .obj file
    GLuint vertex_position_buffer;
    glGenBuffers(1, &vertex_position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_position_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Point3f) * g_cube->NF() * 3, vertex_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(cube_vertex_position_location);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_position_buffer);
    glVertexAttribPointer(cube_vertex_position_location, 3, GL_FLOAT, GL_FALSE, 0, 0);

    free(vertex_data);
}

void setupSphereBuffers(){
    Point3f *vertex_data = (Point3f *)malloc(sizeof(Point3f) * g_sphere->NF() * 3);
    Point3f *normal_data = (Point3f *)malloc(sizeof(Point3f) * g_sphere->NF() * 3);
    for(int i = 0; i < g_sphere->NF(); i++){
        vertex_data[i * 3 + 0] = g_sphere->V(g_sphere->F(i).v[0]);
        vertex_data[i * 3 + 1] = g_sphere->V(g_sphere->F(i).v[1]);
        vertex_data[i * 3 + 2] = g_sphere->V(g_sphere->F(i).v[2]);

        normal_data[i * 3 + 0] = g_sphere->VN(g_sphere->FN(i).v[0]);
        normal_data[i * 3 + 1] = g_sphere->VN(g_sphere->FN(i).v[1]);
        normal_data[i * 3 + 2] = g_sphere->VN(g_sphere->FN(i).v[2]);
    }

    //Generate a vertex buffer
    //and set its data using the vertices read from .obj file
    GLuint vertex_position_buffer;
    glGenBuffers(1, &vertex_position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_position_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Point3f) * g_sphere->NF() * 3, vertex_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(sphere_vertex_position_location);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_position_buffer);
    glVertexAttribPointer(sphere_vertex_position_location, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //Generate a normal buffer
    GLuint vertex_normal_buffer;
    glGenBuffers(1, &vertex_normal_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_normal_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Point3f) * g_sphere->NF() * 3, normal_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(sphere_vertex_normal_location);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_normal_buffer);
    glVertexAttribPointer(sphere_vertex_normal_location, 3, GL_FLOAT, GL_FALSE, 0, 0);

    free(vertex_data);
    free(normal_data);
}

inline void setCubeMap(){
    string cubemap_negx_filename = "../cubemap/cubemap_negx.png";
    string cubemap_negy_filename = "../cubemap/cubemap_negy.png";
    string cubemap_negz_filename = "../cubemap/cubemap_negz.png";
    string cubemap_posx_filename = "../cubemap/cubemap_posx.png";
    string cubemap_posy_filename = "../cubemap/cubemap_posy.png";
    string cubemap_posz_filename = "../cubemap/cubemap_posz.png";

    vector<unsigned char> cubemap_negx_image;
    vector<unsigned char> cubemap_negy_image;
    vector<unsigned char> cubemap_negz_image;
    vector<unsigned char> cubemap_posx_image;
    vector<unsigned char> cubemap_posy_image;
    vector<unsigned char> cubemap_posz_image;

    unsigned width, height;
    unsigned error = lodepng::decode(cubemap_negx_image, width, height, cubemap_negx_filename.c_str());
    if(error){
        cout << "decoder error " << error << ":" << lodepng_error_text(error) << endl;
    }
    error = lodepng::decode(cubemap_negy_image, width, height, cubemap_negy_filename.c_str());
    if(error){
        cout << "decoder error" << error << ":" << lodepng_error_text(error) << endl;
    }
    error = lodepng::decode(cubemap_negz_image, width, height, cubemap_negz_filename.c_str());
    if(error){
        cout << "decoder error" << error << ":" << lodepng_error_text(error) << endl;
    }
    error = lodepng::decode(cubemap_posx_image, width, height, cubemap_posx_filename.c_str());
    if(error){
        cout << "decoder error " << error << ":" << lodepng_error_text(error) << endl;
    }
    error = lodepng::decode(cubemap_posy_image, width, height, cubemap_posy_filename.c_str());
    if(error){
        cout << "decoder error " << error << ":" << lodepng_error_text(error) << endl;
    }
    error = lodepng::decode(cubemap_posz_image, width, height, cubemap_posz_filename.c_str());
    if(error){
        cout << "decoder error " << error << ":" << lodepng_error_text(error) << endl;
    }

    assert(cubemap_negx_image.size() == 2048 * 2048 * 4);
    assert(cubemap_negy_image.size() == 2048 * 2048 * 4);
    assert(cubemap_negz_image.size() == 2048 * 2048 * 4);
    assert(cubemap_posx_image.size() == 2048 * 2048 * 4);
    assert(cubemap_posy_image.size() == 2048 * 2048 * 4);
    assert(cubemap_posz_image.size() == 2048 * 2048 * 4);

    GLubyte *cubemap_negx_data = (GLubyte *)malloc(sizeof(GLubyte) * 2048 * 2048 * 4);
    GLubyte *cubemap_negy_data = (GLubyte *)malloc(sizeof(GLubyte) * 2048 * 2048 * 4);
    GLubyte *cubemap_negz_data = (GLubyte *)malloc(sizeof(GLubyte) * 2048 * 2048 * 4);
    GLubyte *cubemap_posx_data = (GLubyte *)malloc(sizeof(GLubyte) * 2048 * 2048 * 4);
    GLubyte *cubemap_posy_data = (GLubyte *)malloc(sizeof(GLubyte) * 2048 * 2048 * 4);
    GLubyte *cubemap_posz_data = (GLubyte *)malloc(sizeof(GLubyte) * 2048 * 2048 * 4);

    for(int i = 0; i < cubemap_negx_image.size(); i++){
        cubemap_negx_data[i] = cubemap_negx_image[i];
        cubemap_negy_data[i] = cubemap_negy_image[i];
        cubemap_negz_data[i] = cubemap_negz_image[i];
        cubemap_posx_data[i] = cubemap_posx_image[i];
        cubemap_posy_data[i] = cubemap_posy_image[i];
        cubemap_posz_data[i] = cubemap_posz_image[i];
    }

    g_cubemap.Initialize();
    g_cubemap.SetImageRGBA(cy::GLTextureCubeMap::POSITIVE_X, cubemap_posx_data, width, height);
    g_cubemap.SetImageRGBA(cy::GLTextureCubeMap::POSITIVE_Y, cubemap_posy_data, width, height);
    g_cubemap.SetImageRGBA(cy::GLTextureCubeMap::POSITIVE_Z, cubemap_posz_data, width, height);
    g_cubemap.SetImageRGBA(cy::GLTextureCubeMap::NEGATIVE_X, cubemap_negx_data, width, height);
    g_cubemap.SetImageRGBA(cy::GLTextureCubeMap::NEGATIVE_Y, cubemap_negy_data, width, height);
    g_cubemap.SetImageRGBA(cy::GLTextureCubeMap::NEGATIVE_Z, cubemap_negz_data, width, height);
    g_cubemap.BuildMipmaps();
    g_cubemap.SetFilteringMode(GL_NEAREST, GL_LINEAR);

    g_cubemap.Bind(0);

    free(cubemap_negx_data);
    free(cubemap_negy_data);
    free(cubemap_negz_data);
    free(cubemap_posx_data);
    free(cubemap_posy_data);
    free(cubemap_posz_data);
}

void onDisplay(){
    //g_render_buffer->Bind();
    //glUseProgram(g_teapot_program->GetID());
    //glBindVertexArray(g_teapot_VAO);
    //glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    //glEnableVertexAttribArray(teapot_vertex_position_location);
    //glEnableVertexAttribArray(teapot_vertex_normal_location);
    //glEnableVertexAttribArray(teapot_vertex_texcoord_location);
    //glDrawArrays(GL_TRIANGLES, 0, g_mesh->NF() * 3);
    //g_render_buffer->Unbind();

    /*glDepthFunc(GL_ALWAYS);
    glUseProgram(g_cube_program->GetID());
    glBindVertexArray(g_cube_VAO);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnableVertexAttribArray(cube_vertex_position_location);
    glDrawArrays(GL_TRIANGLES, 0, g_cube->NF() * 3);

    glDepthFunc(GL_LESS);
    glUseProgram(g_sphere_program->GetID());
    glBindVertexArray(g_sphere_VAO);
    glEnableVertexAttribArray(sphere_vertex_position_location);
    glEnableVertexAttribArray(sphere_vertex_normal_location);
    glDrawArrays(GL_TRIANGLES, 0, g_sphere->NF() * 3);*/

    glDepthFunc(GL_ALWAYS);
    glUseProgram(g_plane_program->GetID());
    glBindVertexArray(g_plane_VAO);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnableVertexAttribArray(plane_vertex_position_location);
    glEnableVertexAttribArray(plane_vertex_normal_location);
    glEnableVertexAttribArray(plane_vertex_texcoord_location);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glfwSwapBuffers(g_window);
}



void cursor_position_callback(GLFWwindow *window, double xpos, double ypos){
    /*if(g_plane_angle_record){
        Matrix4<float> rz;
        rz.SetRotation(Point3f(1, 0, 0), ypos - g_plane_angle_record_coord.y);
        Matrix4<float> rx;
        rx.SetRotation(Point3f(0, 1, 0), xpos - g_plane_angle_record_coord.x);
        g_plane_mouse_rotation_matrix = rx * rz;
    }
    if(g_plane_dist_record){
        g_plane_dist = ypos - g_plane_dist_record_coord.y;
    }
    if(g_teapot_angle_record){
        Matrix4<float> rz;
        rz.SetRotation(Point3f(1, 0, 0), ypos - g_teapot_angle_record_coord.y);
        Matrix4<float> rx;
        rx.SetRotation(Point3f(0, 1, 0), xpos - g_teapot_angle_record_coord.x);
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
    setTeapotModelViewProjectionMatrix();*/
    if(g_cube_angle_record){
        Matrix4<float> rz;
        rz.SetRotation(Point3f(1, 0, 0), (ypos - g_cube_angle_record_coord.y) * 3.14 / 180);
        Matrix4<float> rx;
        rx.SetRotation(Point3f(0, 1, 0), (xpos - g_cube_angle_record_coord.x) * 3.14 / 180);
        g_cube_mouse_rotation_matrix = rx * rz;
        g_sphere_mouse_rotation_matrix = rx * rz;
        g_sphere_mouse_rotation_matrix.Invert();
    }
    setCubeModelViewProjectionMatrix();
    setSphereModelViewProjectionMatrix();
}

void mouse_button_callback(GLFWwindow *window,int button, int action, int mods){
    double xpos, ypos;
    glfwGetCursorPos(g_window, &xpos, &ypos);
    /*if(button == GLFW_MOUSE_BUTTON_RIGHT){
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
    setTeapotModelViewProjectionMatrix();*/

    if(button == GLFW_MOUSE_BUTTON_RIGHT){
        if(action == GLFW_PRESS){
            if(!g_cube_angle_record){
                g_cube_angle_record_coord = Point2f(xpos, ypos);
                g_cube_angle_record = true;
            }
        }
        else{
            g_cube_angle_record = false;
        }
    }
    setCubeModelViewProjectionMatrix();
}

static void error_callback(int error, const char* description){
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
  if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
      glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

inline bool renderPlane(){
    glGenVertexArrays(1, &g_plane_VAO);
    glBindVertexArray(g_plane_VAO);

    g_plane_program = new GLSLProgram();
    g_plane_program->BuildFiles("../glsl/plane.vert", "../glsl/plane.frag");
    g_plane_program->RegisterUniform(0, "modelViewProjection");
    g_plane_program->RegisterUniform(1, "normalTransform");
    g_plane_program->RegisterUniform(2, "modelView");
    g_plane_program->Bind();
    plane_vertex_position_location = glGetAttribLocation(g_plane_program->GetID(), "pos");
    plane_vertex_normal_location = glGetAttribLocation(g_plane_program->GetID(), "inputNormal");
    plane_vertex_texcoord_location = glGetAttribLocation(g_plane_program->GetID(), "inputTexCoord");
    setPlaneModelViewProjectionMatrix();
    setupPlaneBuffers();

    setCubeMap();
    //g_cubemap.Bind(0);

    //textures
    //g_render_buffer->Initialize(true);
    //g_render_buffer->Resize(4, g_screen_width, g_screen_height);
   //if(!g_render_buffer->IsComplete()){
    //    printf("buffer not complete\n"); return false;
    //}

    //g_render_buffer->BindTexture(4);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    //GLint texLoc = glGetUniformLocation(g_plane_program->GetID(), "map_Kd");
    //glUniform1i(texLoc, g_render_buffer->GetTextureID());
    //g_render_buffer->BuildTextureMipmaps();
    //g_render_buffer->SetTextureMaxAnisotropy();
    //g_render_buffer->SetTextureFilteringMode(GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST);

    return true;
}

inline void renderTeapot() {
    //Generate and bind a vertex array object
    glGenVertexArrays(1, &g_teapot_VAO);
    glBindVertexArray(g_teapot_VAO);

    g_teapot_program = new GLSLProgram();
    g_teapot_program->BuildFiles("../glsl/teapot.vert", "../glsl/teapot.frag");
    g_teapot_program->RegisterUniform(0, "lightPosition");
    g_teapot_program->RegisterUniform(1, "modelViewProjection");
    g_teapot_program->RegisterUniform(2, "normalTransform");
    g_teapot_program->RegisterUniform(3, "modelView");
    g_teapot_program->RegisterUniform(4, "Ns");
    g_teapot_program->RegisterUniform(5, "Ka");
    g_teapot_program->RegisterUniform(6, "Kd");
    g_teapot_program->RegisterUniform(7, "Ks");

    g_teapot_program->Bind();
    teapot_vertex_position_location = glGetAttribLocation(g_teapot_program->GetID(), "pos");
    teapot_vertex_normal_location = glGetAttribLocation(g_teapot_program->GetID(), "inputNormal");
    teapot_vertex_texcoord_location = glGetAttribLocation(g_teapot_program->GetID(), "inputTexCoord");

    Point3f p_Ka; p_Ka.Set(g_mesh->M(0).Ka);
    Point3f p_Kd; p_Kd.Set(g_mesh->M(0).Kd);
    Point3f p_Ks; p_Ks.Set(g_mesh->M(0).Ks);

    g_teapot_program->SetUniform(4, g_mesh->M(0).Ns);
    g_teapot_program->SetUniform(5, p_Ka.x, p_Ka.y, p_Ka.z);
    g_teapot_program->SetUniform(6, p_Kd.x, p_Kd.y, p_Kd.z);
    g_teapot_program->SetUniform(7, p_Ks.x, p_Ks.y, p_Ks.z);
    setTeapotModelViewProjectionMatrix();
    setupTeapotBuffers();

    //textures
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

    GLTexture2D map_Ka;
    map_Ka.Initialize();
    map_Ka.SetImageRGBA(map_Ka_data,width, height);
    map_Ka.BuildMipmaps();
    map_Ka.SetWrappingMode(GL_REPEAT, GL_REPEAT);
    map_Ka.SetFilteringMode(GL_NEAREST, GL_LINEAR);

    GLTexture2D map_Kd;
    map_Kd.Initialize();
    map_Kd.SetImageRGBA(map_Kd_data, width, height);
    map_Kd.BuildMipmaps();
    map_Kd.SetWrappingMode(GL_REPEAT, GL_REPEAT);
    map_Kd.SetFilteringMode(GL_NEAREST, GL_LINEAR);

    GLTexture2D map_Ks;
    map_Ks.Initialize();
    map_Ks.SetImageRGBA(map_Ks_data, width, height);
    map_Ks.BuildMipmaps();
    map_Ks.SetWrappingMode(GL_REPEAT, GL_REPEAT);
    map_Ks.SetFilteringMode(GL_NEAREST, GL_LINEAR);

    GLint texLoc = glGetUniformLocation(g_teapot_program->GetID(), "map_Ka");
    glUniform1i(texLoc, map_Ka.GetID());
    texLoc = glGetUniformLocation(g_teapot_program->GetID(), "map_Kd");
    glUniform1i(texLoc, map_Kd.GetID());
    texLoc = glGetUniformLocation(g_teapot_program->GetID(), "map_Ks");
    glUniform1i(texLoc, map_Ks.GetID());

    map_Ka.Bind(0);
    map_Kd.Bind(1);
    map_Ks.Bind(2);

    free(map_Ka_data);
    free(map_Kd_data);
    free(map_Ks_data);

}

inline void renderCube() {
    g_cube = new TriMesh();
    if(!g_cube->LoadFromFileObj("../cube.obj", false)){
        cerr << "failure of loading the obj file" << endl;
    }

    //Generate and bind a vertex array object
    glGenVertexArrays(1, &g_cube_VAO);
    glBindVertexArray(g_cube_VAO);

    g_cube_program = new GLSLProgram();
    g_cube_program->BuildFiles("../glsl/cube.vert", "../glsl/cube.frag");
    g_cube_program->RegisterUniform(0, "modelViewProjection");

    g_cube_program->Bind();
    cube_vertex_position_location = glGetAttribLocation(g_cube_program->GetID(), "pos");

    setCubeModelViewProjectionMatrix();
    setupCubeBuffers();
    setCubeMap();
}



inline void renderSphere(){
    g_render_buffer = new GLRenderBuffer2D();

    g_sphere = new TriMesh();
    if(!g_sphere->LoadFromFileObj("../teapot.obj", true)){
        cerr << "failure of loading the obj file" << endl;
    }
    g_sphere->ComputeNormals(false);

    //Generate and bind a vertex array object
    glGenVertexArrays(1, &g_sphere_VAO);
    glBindVertexArray(g_sphere_VAO);

    g_sphere_program = new GLSLProgram();
    g_sphere_program->BuildFiles("../glsl/sphere.vert", "../glsl/sphere.frag");
    g_sphere_program->RegisterUniform(0, "lightPosition");
    g_sphere_program->RegisterUniform(1, "modelViewProjection");
    g_sphere_program->RegisterUniform(2, "normalTransform");
    g_sphere_program->RegisterUniform(3, "modelView");
    g_sphere_program->RegisterUniform(4, "Ns");
    g_sphere_program->RegisterUniform(5, "Ka");
    g_sphere_program->RegisterUniform(6, "Kd");
    g_sphere_program->RegisterUniform(7, "Ks");

    g_sphere_program->Bind();
    sphere_vertex_position_location = glGetAttribLocation(g_sphere_program->GetID(), "pos");
    sphere_vertex_normal_location = glGetAttribLocation(g_sphere_program->GetID(), "inputNormal");

    Point3f p_Ka; p_Ka.Set(g_sphere->M(0).Ka);
    Point3f p_Kd; p_Kd.Set(g_sphere->M(0).Kd);
    Point3f p_Ks; p_Ks.Set(g_sphere->M(0).Ks);

    g_sphere_program->SetUniform(4, g_sphere->M(0).Ns);
    g_sphere_program->SetUniform(5, p_Ka.x, p_Ka.y, p_Ka.z);
    g_sphere_program->SetUniform(6, p_Kd.x, p_Kd.y, p_Kd.z);
    g_sphere_program->SetUniform(7, p_Ks.x, p_Ks.y, p_Ks.z);

    setSphereModelViewProjectionMatrix();
    setupSphereBuffers();
    g_cubemap.Bind(0);

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
    g_teapot_dist = (g_maxV - g_minV).z * 3;
    g_plane_mouse_rotation_matrix.SetIdentity();
    g_teapot_mouse_rotation_matrix.SetIdentity();
    g_cube_mouse_rotation_matrix.SetIdentity();
    g_sphere_mouse_rotation_matrix.SetIdentity();

    //GLFW window
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()){
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    g_window = glfwCreateWindow(g_screen_width, g_screen_height, "Project 6 of cs6610", NULL, NULL);
    if (!g_window){
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwSetKeyCallback(g_window, key_callback);
    //glfwSetCursorPosCallback(g_window, cursor_position_callback);
    //glfwSetMouseButtonCallback(g_window, mouse_button_callback);
    glfwMakeContextCurrent(g_window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);

    //renderTeapot();
    //renderCube();
    //renderSphere();
    renderPlane();

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
