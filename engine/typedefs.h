#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <variant>
#include <string>

namespace engine {

typedef std::variant<float, int, bool, std::string> Property;

}

#endif