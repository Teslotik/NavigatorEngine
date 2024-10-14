#ifndef IMAGE_H
#define IMAGE_H

namespace engine::data {

struct Image {
    unsigned id = 0;
    int w, h, channels;
};

}; // namespace engine::data

#endif