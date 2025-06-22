//
// Created by kalmbacj on 6/22/25.
//

#include "./IndirectIota.h"
#include <iostream>
//#include <print>


int main() {
    size_t i = 10'000'000;
    size_t result = 0;
    for (auto& vec: iota_gen_batched_std() | std::views::take(i)) {
        result += vec.back();
        //std::cout << "The back is " << vec.back() << '\n';
        //std::print("{}", vec.size() );
    }
    std::cout << "The result is " << result << '\n';
    return result;
}

