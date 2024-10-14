#ifndef OBJ_PARSER_H
#define OBJ_PARSER_H

#include <iostream>
#include <vector>
#include <numeric>
#include <string>
#include <sstream>
#include <fstream>
#include <iterator>

namespace engine::format::obj {

using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::accumulate;
using std::pair;
using std::stringstream;
using std::getline;
using std::stof;
using std::stoi;

enum class Token {
    Unknown,
    Space,
    Separator,  // /
    EndOfLIne,
    Comment,
    Value
};

enum class Type {
    Unknown,
    Vertex,
    Uv,
    Normal,
    Parameter,
    Face,
    Group,
    Object,
    Library,
    Material,
    Smooth,
    Line
};

struct FaceElement {
    int vertex = -1;
    int uv = -1;
    int normal = -1;
};

/**
 * @details
 * итератор возвращает указатель на элемент в массиве данных
 * поскольку размер вектора может быть разным (2 или 3) также передаётся размер данных
 * итератор увеличивает указатель на элемент в массиве элементов граней на размер элемента граней
**/
template<int S>
class ElementIterator {
public:
    ElementIterator(int *indicesPtr, vector<float> &data): data(data) {
        ptr = indicesPtr;
    }
    ElementIterator(const ElementIterator &v) = delete;
    ElementIterator(ElementIterator &&v) = delete;

    float &operator*() {
        return data[(*(ptr + i * 3) - 1) * S];
    }

    float *operator->() {
        return &data[(*(ptr + i * 3) - 1) * S];
    }

    ElementIterator &operator++() {
        i++;
        return *this;
    }

    friend bool operator==(const ElementIterator& a, const ElementIterator& b) {
        return a.ptr + a.i * 3 == b.ptr + b.i * 3;
    }

    friend bool operator!=(const ElementIterator& a, const ElementIterator& b) {
        return a.ptr + a.i * 3 != b.ptr + b.i * 3;
    };

private:
    int i = 0;
    int *ptr;
    vector<float> &data;
};

struct Object {
    string name;
    vector<unsigned> indices;   // индексы элементов граней
    vector<float> vertices;
    vector<float> uvs;
    vector<float> normals;
    vector<unsigned> materials;    /// @todo
    vector<float> lines;
    bool smooth = false;
    
    vector<FaceElement> faces;

    ElementIterator<3> verticesBegin() {
        return ElementIterator<3>(&faces.data()->vertex, vertices); /// @todo front
    }

    ElementIterator<3> verticesEnd() {
        return ElementIterator<3>(&faces.back().vertex + 3, vertices);
    }

    // ElementIterator uvsBegin() {
    //     return ElementIterator(&faces.data()->uv, uvs);
    // }

    // ElementIterator uvsEnd() {
    //     return ElementIterator(&faces.data()->uv + faces.size() * sizeof(FaceElement), uvs);
    // }

    ElementIterator<3> normalsBegin() {
        return ElementIterator<3>(&faces.data()->normal, normals);
    }

    ElementIterator<3> normalsEnd() {
        return ElementIterator<3>(&faces.back().normal + 3, normals);
    }

    /// @todo
    // MaterialIterator materialsBegin() {
    //     return MaterialIterator(&faces.data()->normal, materials);
    // }

    // MaterialIterator materialsEnd() {
    //     return MaterialIterator(&faces.data()->normal + faces.size() * sizeof(FaceElement), materials);
    // }

    std::vector<unsigned>::iterator materialsBegin() {
        return materials.begin();
    }

    std::vector<unsigned>::iterator materialsEnd() {
        return materials.end();
    }
};


struct Group {

};


struct Data {
    vector<Object> objects;
    vector<Group> groups;
    vector<string> materials;
};


struct Parser {
protected:
    Data *data = new Data();

    int start = 0;
    stringstream ss;

    unsigned indexOffset = 0;
    unsigned vertexOffset = 0;
    unsigned uvOffset = 0;
    unsigned normalOffset = 0;
    unsigned material = 0;

public:
    Parser() = default;

    ~Parser() {
        delete data;
    }

    pair<Token, string> nextToken(string line) {
        ss.str(string());
        Token token = Token::Unknown;
        for (auto i = line.begin() + start; i < line.end(); i++) {
            char c = *i;
            
            // Determinating current token
            Token current = Token::Unknown;
            if (c == '#') {
                current = Token::Comment;
            } else if (c == ' ') {
                current = Token::Space;
            } else if (c == '/') {
                current = Token::Separator;
            } else if (c == '\n') {
                current = Token::EndOfLIne;
            } else {
                current = Token::Value;
            }

            // Is token found
            if (current == Token::Comment) {
                if (token == Token::Unknown) token = Token::Comment;
                break;
            }
            if (token != Token::Unknown && (current != token || current == Token::Separator)) {
                break;
            }

            // Collecting data
            ss << c;
            token = current;
            start++;
        }
        return {token, ss.str()};
    }

    static const char *getTokenName(Token token) {
        switch (token) {
            case Token::Unknown: return "Unknown";
            case Token::Space: return "Space";
            case Token::Separator: return "Separator";
            case Token::EndOfLIne: return "EndOfLIne";
            case Token::Comment: return "Comment";
            // case Token::Name: return "Name";
            case Token::Value: return "Value";
        }
        return "NotFound";
    }

    static const char *getTypeName(Type type) {
        switch (type) {
            case Type::Unknown: return "Unknown";
            case Type::Vertex: return "Vertex";
            case Type::Uv: return "Uv";
            case Type::Normal: return "Normal";
            case Type::Parameter: return "Parameter";
            case Type::Face: return "Face";
            case Type::Group: return "Group";
            case Type::Object: return "Object";
            case Type::Library: return "Library";
            case Type::Material: return "Material";
            case Type::Smooth: return "Smooth";
            case Type::Line: return "Line";
        }
        return "NotFound";
    }

    Data *getData() {
        return data;
    }

    void triangulate(Object *object, vector<FaceElement> *elements) {
        // Fan triangulation
        int o = object->faces.size();
        for (int i = 0; i < elements->size() - 2; i++) {
            object->indices.emplace_back(o);
            object->indices.emplace_back(o + i + 1);
            object->indices.emplace_back(o + i + 2);
        }

        for (auto it: *elements) {
            it.vertex -= vertexOffset;
            it.uv -= uvOffset;
            it.normal -= normalOffset;
            object->faces.push_back(it);
            object->materials.push_back(material);
        }
    }

    void parse(string line) {
        Type type = Type::Unknown;
        Object *currentObject = data->objects.empty() ? nullptr : &data->objects.back();
        Group *currentGroup = data->groups.empty() ? nullptr : &data->groups.back();
        vector<FaceElement> currentFace;
        FaceElement *currentFaceElement = nullptr;
        int item = 0;   // face element item
        start = 0;
        bool newline = true;
        while (start < line.length()) {
            auto [token, str] = nextToken(line);
            if (token == Token::Unknown || token == Token::EndOfLIne || token == Token::Comment) {
                break;
            }
            
            // cout << "Token:" << getTokenName(token) << " " << str << endl;

            if (type == Type::Unknown) {
                if (str == "v") {
                    type = Type::Vertex;
                } else if (str == "vt") {
                    type = Type::Uv;
                } else if (str == "vn") {
                    type = Type::Normal;
                } else if (str == "vp") {
                    type = Type::Parameter;
                } else if (str == "f") {
                    type = Type::Face;
                } else if (str == "g") {
                    type = Type::Group;
                } else if (str == "o") {
                    type = Type::Object;
                } else if (str == "mtllib") {
                    type = Type::Library;
                } else if (str == "usemtl") {
                    type = Type::Material;
                } else if (str == "s") {
                    type = Type::Smooth;
                } else if (str == "l") {
                    type = Type::Line;
                } else {
                    cout << "[Warning] Unknown type: " << str << endl;
                    break;
                }
                newline = false;
                continue;
            }

            if (type == Type::Face) {
                if (newline) {
                    currentFace = vector<FaceElement>();
                } else if (token == Token::Space) {
                    currentFaceElement = &currentFace.emplace_back();
                    item = 0;
                } else if (token == Token::Separator) {
                    item++;
                } else if (token == Token::Value) {
                    if (item == 0) {
                        currentFaceElement->vertex = stoi(str);
                    } else if (item == 1) {
                        currentFaceElement->uv = stoi(str);
                    } else if (item == 2) {
                        currentFaceElement->normal = stoi(str);
                    } else {
                        cout << "[Warning] Too much face elements" << endl;
                    }
                }
            } else if (token == Token::Space) {
                continue;
            } else if (type == Type::Vertex) {
                currentObject->vertices.push_back(stof(str));
            } else if (type == Type::Uv) {
                currentObject->uvs.push_back(stof(str));
            } else if (type == Type::Normal) {
                currentObject->normals.push_back(stof(str));
            } else if (type == Type::Parameter) {
                cout << "[Warning] Parameter space vertices is not supported" << endl;
            } else if (type == Type::Group) {
                data->groups.emplace_back();
            } else if (type == Type::Object) {
                if (currentObject != nullptr) {
                    indexOffset -= currentObject->faces.size();
                    vertexOffset += currentObject->vertices.size() / 3;
                    uvOffset += currentObject->uvs.size() / 2;
                    normalOffset += currentObject->normals.size() / 3;
                }
                Object &object = data->objects.emplace_back();
                object.name = str;
            } else if (type == Type::Library) {
            } else if (type == Type::Material) {
                /// @note the wrong approach, because data::materials must contain name of the libraries, not the materials name
                /// @todo add materials parser
                auto it = std::find(data->materials.begin(), data->materials.end(), str);
                if (it != data->materials.end()) {
                    material = std::distance(data->materials.begin(), it);
                } else {
                    data->materials.push_back(str);
                    material = data->materials.size() - 1;
                }
            } else if (type == Type::Smooth) {
                currentObject->smooth = str == "1" || str == "on";
            } else if (type == Type::Line) {
                currentObject->lines.push_back(stof(str));
            }

            newline = false;
        }
        if (type == Type::Face) {
            triangulate(currentObject, &currentFace);
        }
    }
};

} // namespace engine::format::obj

#endif