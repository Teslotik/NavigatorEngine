#ifndef LOADER_H
#define LOADER_H

#include <string>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <functional>
#include <json.hpp>

#include "platform.h"
#include "data/Material.h"
#include "entity/MeshActor.h"
#include "entity/Shader.h"
#include "data/Image.h"

namespace engine {

struct Application;

class File {
protected:
    std::ifstream *stream = nullptr;    // @todo read/write

public:
    File(std::filesystem::path path) {
        assert(std::filesystem::exists(path));

        stream = new std::ifstream();
        stream->exceptions(std::ifstream::badbit);

        try {
            stream->open(path, std::ios::binary);
        } catch (std::ifstream::failure &e) {
            std::cout << "[Error] Failed to load file stream: " << path << " Exception: " << e.what() << std::endl;
            delete stream;
            stream = nullptr;
        }
    }

    ~File() {
        if (stream) {
            stream->close();
            delete stream;
        }
    }

    File(const File &o) = delete;
    
    File(File &&o) {
        stream = o.stream;
        o.stream = nullptr;
    }

    File &operator=(const File &o) = delete;

    File &operator=(File &&o) {
        stream = o.stream;
        o.stream = nullptr;
        return *this;
    }

    std::ifstream *getStream() {
        return stream;
    }
};

#if defined(LINUX) || defined(WINDOWS)
/// @todo also add UserStorage, EngineStorage, ConfigStorage, PersistentStorage?
/// @note can be made using CRTP // class BaseLoader
class Loader {
    std::filesystem::path path = "tmp";
public:
    Loader() = default;
    Loader(std::filesystem::path path);

    File getFile(std::string label);

    // void open()  /// @todo requests permissions

    void stream(std::string label, std::function<void(std::ifstream&)> reader);
    std::string text(std::string label);
    nlohmann::json json(std::string label);
    std::string request(const char *url);
    std::vector<entity::MeshActor*> mesh(std::string label, std::map<std::string, engine::data::Material> *materials);  /// @todo remove Material
    entity::Shader *shader(const char *vertex, const char *fragment);
    // pkpy::VM *createPythonVm(Application *app);
    // void python(pkpy::VM *vm, std::string label);
    data::Image image(std::string label);
};
#endif

} // namespace engine

#endif