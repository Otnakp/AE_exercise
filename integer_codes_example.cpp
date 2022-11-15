#include<iostream>
#include <random>
#include<sdsl/vectors.hpp> // it can represent 0 but uses 1 bit more per number

void example1(){
    // to random access keep another array of "pointers"
    // so that you can skip directly to that one
    sdsl::int_vector<> v(10 * (1 << 20));
    std::cout << (1 << 2) << std::endl; // 4 because this is 0b00000010
    std::cout << (10 * (1 << 2)) << std::endl; // 10 * 4
    std::cout << (10 * (1 << 20)) << std::endl; // 10 * 2^(20)
    v[100] = 1ULL << 63; // set the max to use 64 bits
    std::cout << (1ULL << 63) << std::endl; // 1ULL stands for unsigned long long, so is a lot of 1s
    sdsl::util::bit_compress(v);
    std::cout << sdsl::size_in_mega_bytes(v) << std::endl;
    sdsl::vlc_vector<sdsl::coder::elias_delta, 128> vlc(v); // 128 is the density (store pointers to do random access)
    std::cout << sdsl::size_in_mega_bytes(vlc) << std::endl;
}

sdsl::int_vector<> random_data(size_t n, uint64_t u){
    if(n>u)
        throw std::invalid_argument("n>u");
    std::mt19937 rnd(std::random_device{}());
    sdsl::int_vector<> out(n);
    for(size_t j=1, s = 0; j<=u && s<n; ++j){
        if(rnd()%(u-j+1)<n-s){
            out[s++] = j;
        }
    }
    return out;
}

void example2(){
    // to random access keep an array with the "pointers" and the sum until that point (because you use gap)
    // gap code
    // then delta code the gaps
    auto data = random_data(10 * (1 << 20), 1 << 24);
    std::cout << sdsl::size_in_mega_bytes(data) << std::endl;

    sdsl::vlc_vector<sdsl::coder::elias_delta, 128> vlc(data);
    std::cout << sdsl::size_in_mega_bytes(vlc) << std::endl;

    sdsl::enc_vector<sdsl::coder::elias_delta, 128> env_delta(data);
    std::cout << sdsl::size_in_mega_bytes(env_delta) << std::endl;

    //if you do gamma code you will use less space probably because gamma is better than delta for small numbers
}

int main(){
    example1();
    std::cout << "________________________________________________" << std::endl;
    example2();
}