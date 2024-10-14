#include "Tag.h"
#include "utils.inl"
#include "interface/IProcessable.h"
#include "functional"

using namespace engine::entity;
using namespace std;

void Tag::input(Pipeline *pipeline) {
    if (script) script->input(pipeline);
}

void Tag::update(Pipeline *pipeline) {
    if (script) script->update(pipeline);
    tween.update(pipeline->deltaTime);
}

void Tag::fixedUpdate(Pipeline *pipeline) {
    if (script) script->fixedUpdate(pipeline);
}