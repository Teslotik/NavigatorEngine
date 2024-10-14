#include <iostream>
#include <sstream>
#include <fstream>

// #define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Loader.h"
#include "format/Obj.h"
#include "entity/MeshActor.h"
#include "entity/Shader.h"
// #include "api/PythonApi.h"
// #include "Application.h"

using std::stringstream;
using std::ifstream;
using std::getline;
using std::cout;
using std::endl;
using namespace engine;
using entity::MeshActor;
using entity::Shader;
using data::Material;
using data::Image;

using std::ifstream;
using std::string;
using std::function;
using std::vector;
using engine::entity::Shader;

Loader::Loader(std::filesystem::path path): path(path) {
    
}

File Loader::getFile(std::string label, bool binary) {
    std::filesystem::path p = path / label;
    // cout << path << ", " << p << endl;
    // cout << std::filesystem::absolute(p) << std::filesystem::current_path() << endl;
    return File(p, binary);
}

string Loader::text(std::string label) {
    File file = getFile(label);
    string source;
    stringstream stream;
    stream << file.getStream()->rdbuf();
    return stream.str();
}

nlohmann::json Loader::json(std::string label) {
    return nlohmann::json::parse(text(label));
}

vector<MeshActor*> Loader::mesh(std::string label, std::map<string, engine::data::Material> *materials) {
    vector<MeshActor*> meshes;
    File file = getFile(label);
    auto stream = file.getStream();

    engine::format::obj::Parser parser;

    string line;
    while (getline(*stream, line)) {
        parser.parse(line);
    }

    cout << "[Info] Meshes: " << parser.getData()->objects.size() << ", in file: " << label << endl;
    for (auto obj: parser.getData()->objects) {
        cout << "[Info] Loading mesh:" << obj.name
            << ", vertices:" << obj.vertices.size() / 3
            << ", normals:" << obj.normals.size() / 3
            << ", indices:" << obj.indices.size() << endl;
        
        float *vertices = new float[obj.faces.size() * (3 + 3 + 4)];
        {
            int offset = 0;
            for (auto it = obj.verticesBegin(); it != obj.verticesEnd(); ++it) {
                /// @todo remove 0.01f
                vertices[offset + 0] = (&*it)[0] * 0.01f;
                vertices[offset + 1] = (&*it)[1] * 0.01f;
                vertices[offset + 2] = (&*it)[2] * 0.01f;
                offset += 10;
            }
        }

        {
            int offset = 3;
            for (auto it = obj.normalsBegin(); it != obj.normalsEnd(); ++it) {
                vertices[offset + 0] = (&*it)[0];
                vertices[offset + 1] = (&*it)[1];
                vertices[offset + 2] = (&*it)[2];
                offset += 10;
            }
        }

        {
            int offset = 6;
            for (auto it = obj.materialsBegin(); it != obj.materialsEnd(); ++it) {
                Material material = (*materials)[parser.getData()->materials[*it]];
                vertices[offset + 0] = material.albedo[0];
                vertices[offset + 1] = material.albedo[1];
                vertices[offset + 2] = material.albedo[2];
                vertices[offset + 3] = material.albedo[3];
                offset += 10;
            }
        }

        auto indices = new unsigned[obj.indices.size()];
        for (int i = 0; i < obj.indices.size(); i++) {
            indices[i] = obj.indices[i];
        }

        meshes.push_back(new MeshActor(vertices, obj.faces.size() * (3 + 3 + 4), indices, obj.indices.size()));
    }
    return meshes;
}

Shader *Loader::shader(const char *vertex, const char *fragment) {
    return new Shader(text(vertex).c_str(), text(fragment).c_str());
}

/// @todo
/*
pkpy::VM *Loader::createPythonVm(Application *app) {
    VM *vm = new VM(false);
    PyVar mod = vm->new_module("engine");
    python::app = app;
    vm->register_user_class<python::PythonApplication>(mod, "Application", VM::tp_object, true);
    vm->register_user_class<python::PythonEntity>(mod, "Entity", VM::tp_object, true);
    return vm;
}

void Loader::python(pkpy::VM *vm, std::string label) {
    vm->exec(R"(
from engine import Application, Entity

entity = Entity.Create('ActorEntity', 'test')
    )"
    );
    // vm->exec("a = engine.Context()");
    // vm->exec("a = engine.Context.__init__(None)");

}
*/

Image Loader::image(std::string label) {
    File file = getFile(label, true);
    
    auto stream = file.getStream();
    stream->seekg(0, stream->end);
    int length = stream->tellg();
    stream->seekg(0, stream->beg);
    char *buffer = new char[length];
    stream->read(buffer, length);
    
    int w, h, channels;
    unsigned char *data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(buffer), length, &w, &h, &channels, 0);
    delete[] buffer;
    /// @todo
    // stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.

    unsigned id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        // glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture" << std::endl;
        return {};
    }
    stbi_image_free(data);
    return {
        .id = id,
        .w = w,
        .h = h,
        .channels = channels
    };
}