#pragma once

#include "Types.h"

struct IComponent
{
protected:
    static int nextId;
};

// Used to assign a unique id to a component type
template<typename T>
class Component : public IComponent
{
public:
    // Returns the unique id of Component<T>
    static int GetId()
    {
        static int id = nextId++;
        return id;
    }
};