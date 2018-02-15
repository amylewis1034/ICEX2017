#ifndef SEAWEED_HPP
#define SEAWEED_HPP

#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include "Component.hpp"

class Seaweed : public Component {
public:
    Seaweed();
    virtual ~Seaweed();

    virtual void init();
    virtual void update(float dt);
    
 private:
   
};

#endif