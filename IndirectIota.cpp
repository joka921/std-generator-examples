//
// Created by kalmbacj on 6/15/25.
//

#include "IndirectIota.h"

size_t IndirectIota::get_next() {
    return i++;
}

class VirtualIota : public Base {
    size_t i = 0;
public:
    size_t get_next() override {
        return i++;
    }
};

std::unique_ptr<Base> makeVirtualIota() {
    return std::make_unique<VirtualIota>();

}

int indirectFunction() {
  return 42;
}