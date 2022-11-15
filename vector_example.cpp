#include<iostream>
#include<sdsl/vectors.hpp>
int main(){
    //sdsl::int_vector<5> v = {3, 8, 31, 13, 0, 9, 4, 26, 31, 100}; // last 1 will be cut because it doesnt fit in 5 bits
    //std::cout << v << std::endl;
    //std::cout << v.bit_size() << std::endl;
    //std::cout << sdsl::size_in_bytes(v) << std::endl;

    sdsl::int_vector<> v = {26, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::cout << sdsl::size_in_bytes(v) << std::endl; // now it will need 89 bytes and not 88 because
    //it needs to remember the width for one number (the one that was 5 before, now is 64)
    sdsl::util::bit_compress(v); // finds the max in v and compresses it using the max num of bits needed
    std::cout << (int)v.width() << std::endl;
    std::cout << sdsl::size_in_bytes(v) << std::endl;
}