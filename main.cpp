#include "Common.h"
#include "OpenGLCommon.h"
#include "OpenGLMarkerObjects.h"
#include "OpenGLBgEffect.h"
#include "OpenGLMesh.h"
#include "OpenGLViewer.h"
#include "OpenGLWindow.h"
#include "TinyObjLoader.h"
#include "OpenGLSkybox.h"
#include <algorithm>
#include <iostream>
#include <random>
#include <unordered_set>
#include <vector>
#include <string>

#ifndef __Main_cpp__
#define __Main_cpp__

#ifdef __APPLE__
#define CLOCKS_PER_SEC 100000
#endif

class MyDriver : public OpenGLViewer
{
    std::vector<OpenGLTriangleMesh *> mesh_object_array;
    OpenGLBgEffect *bgEffect = nullptr;
    OpenGLSkybox *skybox = nullptr;
    clock_t startTime;

public:
    virtual void Initialize()
    {
        draw_axes = false;
        startTime = clock();
        OpenGLViewer::Initialize();
    }

    virtual void Initialize_Data()
    {

        //// Load all the shaders you need for the scene 
        //// In the function call of Add_Shader_From_File(), we specify three names: 
        //// (1) vertex shader file name
        //// (2) fragment shader file name
        //// (3) shader name used in the shader library
        //// When we bind a shader to an object, we implement it as follows:
        //// object->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("shader_name"));
        //// Here "shader_name" needs to be one of the shader names you created previously with Add_Shader_From_File()

        OpenGLShaderLibrary::Instance()->Add_Shader_From_File("shaders/basic.vert", "shaders/basic.frag", "basic");
        OpenGLShaderLibrary::Instance()->Add_Shader_From_File("shaders/basic.vert", "shaders/environment.frag", "environment");
        OpenGLShaderLibrary::Instance()->Add_Shader_From_File("shaders/stars.vert", "shaders/stars.frag", "stars");
        OpenGLShaderLibrary::Instance()->Add_Shader_From_File("shaders/basic.vert", "shaders/alphablend.frag", "blend");
        OpenGLShaderLibrary::Instance()->Add_Shader_From_File("shaders/billboard.vert", "shaders/alphablend.frag", "billboard");
        OpenGLShaderLibrary::Instance()->Add_Shader_From_File("shaders/terrain.vert", "shaders/terrain.frag", "terrain");
        OpenGLShaderLibrary::Instance()->Add_Shader_From_File("shaders/skybox.vert", "shaders/skybox.frag", "skybox");
        //OpenGLShaderLibrary::Instance()->Add_Shader_From_File("shaders/basic.vert", "shaders/water.frag", "water"); 

        //// Load all the textures you need for the scene
        //// In the function call of Add_Shader_From_File(), we specify two names:
        //// (1) the texture's file name
        //// (2) the texture used in the texture library
        //// When we bind a texture to an object, we implement it as follows:
        //// object->Add_Texture("tex_sampler", OpenGLTextureLibrary::Get_Texture("tex_name"));
        //// Here "tex_sampler" is the name of the texture sampler2D you used in your shader, and
        //// "tex_name" needs to be one of the texture names you created previously with Add_Texture_From_File()

        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/earth_color.png", "sphere_color");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/earth_normal.png", "sphere_normal");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/bunny_color.jpg", "bunny_color");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/bunny_normal.png", "bunny_normal");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/window.png", "window_color");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/buzz_color.png", "buzz_color");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/star.png", "star_color");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/boattex.jpg", "boat_color");
        OpenGLTextureLibrary::Instance()->Add_Texture_From_File("tex/water_color.png", "water_color");


        //// Add all the lights you need for the scene (no more than 4 lights)
        //// The four parameters are position, ambient, diffuse, and specular.
        //// The lights you declared here will be synchronized to all shaders in uniform lights.
        //// You may access these lights using lt[0].pos, lt[1].amb, lt[1].dif, etc.
        //// You can also create your own lights by directly declaring them in a shader without using Add_Light().
        //// Here we declared three default lights for you. Feel free to add/delete/change them at your will.

        opengl_window->Add_Light(Vector3f(3, 1, 3), Vector3f(0.1, 0.1, 0.1), Vector3f(1, 1, 1), Vector3f(0.5, 0.5, 0.5)); 
        opengl_window->Add_Light(Vector3f(0, 0, -5), Vector3f(0.1, 0.1, 0.1), Vector3f(0.9, 0.9, 0.9), Vector3f(0.5, 0.5, 0.5));
        opengl_window->Add_Light(Vector3f(-5, 1, 3), Vector3f(0.1, 0.1, 0.1), Vector3f(0.9, 0.9, 0.9), Vector3f(0.5, 0.5, 0.5));

        //// Add the background / environment
        //// Here we provide you with four default options to create the background of your scene:
        //// (1) Gradient color (like A1 and A2; if you want a simple background, use this one)
        //// (2) Programmable Canvas (like A7 and A8; if you consider implementing noise or particles for the background, use this one)
        //// (3) Sky box (cubemap; if you want to load six background images for a skybox, use this one)
        //// (4) Sky sphere (if you want to implement a sky sphere, enlarge the size of the sphere to make it colver the entire scene and update its shaders for texture colors)
        //// By default, Option (2) (Buzz stars) is turned on, and all the other three are commented out.
        
        //// Background Option (1): Gradient color
        /*
        {
            auto bg = Add_Interactive_Object<OpenGLBackground>();
            bg->Set_Color(OpenGLColor(0.1f, 0.1f, 0.1f, 1.f), OpenGLColor(0.3f, 0.1f, .1f, 1.f));
            bg->Initialize();
        }
        */

        //// Background Option (2): Programmable Canvas
        //// By default, we load a GT buzz + a number of stars
        {
            bgEffect = Add_Interactive_Object<OpenGLBgEffect>();
            bgEffect->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("stars"));
            bgEffect->Initialize();
        }
        
        //// Background Option (3): Sky box
        //// Here we provide a default implementation of a sky box; customize it for your own sky box
        /*
        {
            // from https://www.humus.name/index.php?page=Textures
            const std::vector<std::string> cubemap_files{
                "cubemap/posx.jpg",     //// + X
                "cubemap/negx.jpg",     //// - X
                "cubemap/posy.jpg",     //// + Y
                "cubemap/negy.jpg",     //// - Y
                "cubemap/posz.jpg",     //// + Z
                "cubemap/negz.jpg",     //// - Z 
            };
            OpenGLTextureLibrary::Instance()->Add_CubeMap_From_Files(cubemap_files, "cube_map");

            skybox = Add_Interactive_Object<OpenGLSkybox>();
            skybox->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("skybox"));
            skybox->Initialize();
        }
        */

        //// Background Option (4): Sky sphere
        //// Here we provide a default implementation of a textured sphere; customize it for your own sky sphere
        

        //// Here we show an example of adding a mesh with noise-terrain (A6)
        {
            //// create object by reading an obj mesh
            auto terrain = Add_Obj_Mesh_Object("obj/plane.obj");

            //// set object's transform
            Matrix4f r = Matrix4f::Identity();
            Matrix4f s;
            s << 1.0f, 0,    0,   0,
                0,    3.0f, 0,   0,
                0,    0,    1.0f, 0,
                0,    0,    0,   1.0f;
            Matrix4f t;
            t << 1.0f, 0,    0,   0,
                0,    1.0f, 0,   -3.0,
                0,    0,    1.0f, 0,
                0,    0,    0,   1.0f;

            terrain->Set_Model_Matrix(t * s * r);

            //// set object's material
            terrain->Set_Ka(Vector3f(0.1f, 0.1f, 0.1f));
            terrain->Set_Kd(Vector3f(0.7f, 0.7f, 0.7f));
            terrain->Set_Ks(Vector3f(1, 1, 1));
            terrain->Set_Shininess(128.f);

            //// bind shader to object (we do not bind texture for this object because we create noise for texture)
            terrain->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("terrain"));
            terrain->Add_Texture("water_tex", OpenGLTextureLibrary::Get_Texture("water_color"));

        }
        
        //{
        //    auto water = Add_Obj_Mesh_Object("obj/plane.obj");


        //    Matrix4f r = Matrix4f::Identity();
        //    Matrix4f s;
        //    s << 1.0f, 0, 0, 0,
        //        0, 3.0f, 0, 0,
        //        0, 0, 1.0f, 0,
        //        0, 0, 0, 1.0f;
        //    Matrix4f t;
        //    t << 1.0f, 0, 0, 0,
        //        0, 1.0f, 0, -3.0,
        //        0, 0, 1.0f, 0,
        //        0, 0, 0, 1.0f;

        //    terrain->Set_Model_Matrix(t* s* r);

        //    water->Set_Ka(Vector3f(0.0f, 0.05f, 0.1f));
        //    water->Set_Kd(Vector3f(0.0f, 0.2f, 0.4f));
        //    water->Set_Ks(Vector3f(0.8f, 0.8f, 0.8f));
        //    water->Set_Shininess(128.f);

        //    water->Add_Texture("tex_color", OpenGLTextureLibrary::Get_Texture("sphere_color"));
        //    water->Add_Texture("tex_normal", OpenGLTextureLibrary::Get_Texture("sphere_normal"));

        //    water->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("water"));
        //}

        {
        auto boat = Add_Obj_Mesh_Object("obj/boat.obj");


        float angle1 = 90.0f * 3.1415926f / 180.0f;
        float angle2 = 180.0f * 3.1415926f / 180.0f;
        float angle3 = -10.0f * 3.1415926f / 180.0f;

        Matrix4f r1;
            r1 << cos(angle1), -sin(angle1), 0, 0,
            sin(angle1), cos(angle1), 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1;
        Matrix4f r2;
            r2 << cos(angle3), 0, sin(angle3), 0,
               0, 1, 0, 0,
               -sin(angle3), 0, cos(angle3), 0,
               0, 0, 0, 1;
        Matrix4f r3;
            r3 << 1, 0, 0, 0,
                0, cos(angle1), -sin(angle1), 0,
                0, sin(angle1),  cos(angle1), 0,
                0, 0, 0, 1;
        Matrix4f r = r1 *r2 * r3; 
        Matrix4f s;
            s << .0025f, 0, 0, 0,
                0, .0025f, 0, 0,
                0, 0, .0025f, 0,
                0, 0, 0, 1.0f;
        Matrix4f t;
            t << 1.0f, 0, 0, 2.5f,
                0, 1.0f, 0, -0.2f,
                0, 0, 1.0f, -0.10f,
                0, 0, 0, 1.0f;
        boat->Set_Model_Matrix(t * s * r);
        boat->Set_Ka(Vector3f(0.1f, 0.1f, 0.1f));
        boat->Set_Kd(Vector3f(0.6f, 0.6f, 0.6f));
        boat->Set_Ks(Vector3f(0.2f, 0.2f, 0.2f));
        boat->Set_Shininess(64.f);
        boat->Add_Texture("tex_sampler", OpenGLTextureLibrary::Get_Texture("boat_color"));
        boat->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("basic"));
        }


        //// This for-loop updates the rendering model for each object on the list
        for (auto &mesh_obj : mesh_object_array){
            Set_Polygon_Mode(mesh_obj, PolygonMode::Fill);
            Set_Shading_Mode(mesh_obj, ShadingMode::TexAlpha);
            mesh_obj->Set_Data_Refreshed();
            mesh_obj->Initialize();
        }
        Toggle_Play();
    }

    //// add mesh object by reading an .obj file
    OpenGLTriangleMesh *Add_Obj_Mesh_Object(std::string obj_file_name)
    {
        auto mesh_obj = Add_Interactive_Object<OpenGLTriangleMesh>();
        Array<std::shared_ptr<TriangleMesh<3>>> meshes;
        // Obj::Read_From_Obj_File(obj_file_name, meshes);
        Obj::Read_From_Obj_File_Discrete_Triangles(obj_file_name, meshes);

        mesh_obj->mesh = *meshes[0];
        std::cout << "load tri_mesh from obj file, #vtx: " << mesh_obj->mesh.Vertices().size() << ", #ele: " << mesh_obj->mesh.Elements().size() << std::endl;

        mesh_object_array.push_back(mesh_obj);
        return mesh_obj;
    }

    //// add mesh object by reading an array of vertices and an array of elements
    OpenGLTriangleMesh* Add_Tri_Mesh_Object(const std::vector<Vector3>& vertices, const std::vector<Vector3i>& elements)
    {
        auto obj = Add_Interactive_Object<OpenGLTriangleMesh>();
        mesh_object_array.push_back(obj);
        // set up vertices and elements
        obj->mesh.Vertices() = vertices;
        obj->mesh.Elements() = elements;

        return obj;
    }

    //// Go to next frame
    virtual void Toggle_Next_Frame()
    {
        for (auto &mesh_obj : mesh_object_array)
            mesh_obj->setTime(GLfloat(clock() - startTime) / CLOCKS_PER_SEC);

        if (bgEffect){
            bgEffect->setResolution((float)Win_Width(), (float)Win_Height());
            bgEffect->setTime(GLfloat(clock() - startTime) / CLOCKS_PER_SEC);
            bgEffect->setFrame(frame++);
        }

        if (skybox){
            skybox->setTime(GLfloat(clock() - startTime) / CLOCKS_PER_SEC);
        }   

        OpenGLViewer::Toggle_Next_Frame();
    }

    virtual void Run()
    {
        OpenGLViewer::Run();
    }
};

int main(int argc, char *argv[])
{
    MyDriver driver;
    driver.Initialize();
    driver.opengl_window->camera_target = Vector3f(2.5f, -0.42f, 0.225f);
    driver.opengl_window->camera_distance = 0.0f;
    driver.opengl_window->arcball_matrix  = Matrix4f::Identity();


    //camera clipping fix
    static std::function<void(void)> clamp_zoom = [&]()
    {
        float& d = driver.opengl_window->camera_distance;
        if (d < 1.0f) d = 1.0f;
    };
    driver.opengl_window->Set_Idle_Callback(&clamp_zoom);


    float angle = -90.0f * 3.14f / 180.0f;

    Matrix4f R = Matrix4f::Identity();
    R(1,1) = cos(angle);
    R(1,2) = -sin(angle);
    R(2,1) = sin(angle);
    R(2,2) = cos(angle);

    driver.opengl_window->rotation_matrix = R;
    driver.Run();
}

#endif