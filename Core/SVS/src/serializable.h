/*
 Due to circular include dependencies, this interface has to be
 defined in a separate file from serialize.h
*/

#ifndef SERIALIZABLE_H
#define SERIALIZABLE_H

#include <iostream>

class serializable
{
    public:
        virtual void serialize(std::ostream& os) const = 0;
        virtual void unserialize(std::istream& is) = 0;
};

#endif
