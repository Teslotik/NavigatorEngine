#ifndef IPROCESSABLE_H
#define IPROCESSABLE_H

#include "IListenable.h"
#include "Pipeline.h"

namespace engine {

struct IProcessable: IListenable {
    Pipeline *pipeline = nullptr;
};

} // namespace engine

#endif