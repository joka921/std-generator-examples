//
// Created by kalmbacj on 6/15/25.
//

#ifndef STD_GENERATOR_EXAMPLES_INDIRECTIOTA_H
#define STD_GENERATOR_EXAMPLES_INDIRECTIOTA_H

#include <cstddef>
#include <memory>

class IndirectIota {
    size_t i = 0;
public:
    size_t get_next();
};

class Base {
public:
    virtual size_t get_next() = 0;
};

std::unique_ptr<Base> makeVirtualIota();

int indirectFunction();


#endif //STD_GENERATOR_EXAMPLES_INDIRECTIOTA_H
