#include<iostream>
#include <random>
#include<sdsl/vectors.hpp> // it can represent 0 but uses 1 bit more per number
#include<sdsl/bit_vectors.hpp>

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
void example1(){
    // sdsl::bit_vector b = {1,0,1,1,0,1};
    // sdsl::rank_support_v<1> b_rank(&b);
    // b_rank(i) returns the position of one
    sdsl::bit_vector b = {1,0,1,1,0,1};
    sdsl::rank_support_v<1> b_rank(&b); //strucute capable of returning number of 1 up until a point in O(1)
    for (size_t i = 1; i <= b.size(); ++i){
        std::cout << b_rank(i) << " "; // counts the ones before i
    }
    std::cout << std::endl;

    sdsl::select_support_mcl<1> b_select(&b); // structure able to returns the index of the ith 1 in O(1)
    size_t ones = b_rank(b.size());
    for (size_t i= 1; i <= ones; ++i){
        std::cout << b_select(i) << " ";
    }
    std::cout << std::endl;
}

void elias_fano_example(){
    // sd_vector is the elias fano representation, can build from increasing integer vector or from bit vector
    auto data = random_data(10 * (1 << 20), 1 << 25);
    sdsl::sd_vector<> ef(data.begin(), data.end()); // elias fano vector
    std::cout << sdsl::size_in_mega_bytes(data) << std::endl;
    std::cout << sdsl::size_in_mega_bytes(ef) << std::endl;

    // there's a bug with how sdsl does the splitting between high and low. it uses + or - 1 bit or something
}

void elias_fano_example_2(){
    auto access = [&](size_t i)
    { return ef_select(i + 1); }; // access is select the ith "one", +1 because is ith set bit, and i>=1
    auto nextGEQ = [&](uint64_t x)
    { return ef_select(ef_rank(x) + 1); };  
}

int main(){
    example1();
    std::cout << "___________________"<<std::endl;
    elias_fano_example();
}