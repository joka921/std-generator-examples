//
// Created by kalmbacj on 6/22/25.
//

#include "./IndirectIota.h"


int main() {
    size_t i = 10'000'000;
    size_t result = 0;
    for (auto& vec: iota_gen_batched() | std::views::take(i)) {
        result += vec.back();
    }
    return result;
}

