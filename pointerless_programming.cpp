#include <chrono>
#include <iostream> 
#include <sdsl/bit_vectors.hpp>
#include <vector>

std::string get_word(size_t i, char *w, sdsl::select_support_mcl<1> b_select){
    return &w[b_select(i)]; // automatically finds the \0 
}
size_t word_index(const char* target, char *w, sdsl::select_support_mcl<1> b_select, size_t l, size_t r){
        if(r<=l)
            return -1;
        size_t m = (size_t)((r+l)/2);
        int compare = strcmp(target, get_word(m, w, b_select).c_str());
        if(compare == 0)
            return m;
        if(compare > 0){
            return word_index(target, w, b_select, m+1, r);
        }else{
            return word_index(target, w, b_select, l, m);
        }
    }
bool lookup(const std::string &s, char *w, sdsl::select_support_mcl<1> b_select, size_t num_strings, size_t total_lenght_strings_only){
    // go to num_strings / 2, check string. if it's alphabetically before my, recursion right, else 
    // recursion left

    size_t i = word_index(s.c_str(), w, b_select, 0, num_strings -1);
    if(i == -1)
        return false;
    else{
        return true;
    }
}
int main() {
    std::ifstream file("words.txt"); 
    std::string buffer;
    std::getline (file, buffer);
    size_t num_strings = std::stoull(buffer); 
    std::getline (file, buffer);
    size_t total_length_strings_only = std::stoull (buffer);

    sdsl::bit_vector b(total_length_strings_only + num_strings, 0);
    char *w = new char[total_length_strings_only + num_strings]; // for terminating characters
    size_t i = 0;
    while (std::getline (file, buffer)) // ... load the input data into a char* and fill the bitvector
    {
        strcpy(w + i, &buffer[0] + '\0');
        b[i] = 1;
        i += strlen(buffer.c_str()) + 1;
    }
    sdsl::rank_support_v<1> b_rank(&b);
    sdsl::select_support_mcl<1> b_select(&b);

    std::string s;
    while (true)
        {
         std::cout << "input string (return to exit) >";
         std::getline(std::cin, s);
         if (s.empty()){
             break;
         }
         auto t0= std::chrono::high_resolution_clock::now();
         auto flag = lookup (s, w, b_select, num_strings, total_length_strings_only);
         auto t1 = std::chrono::high_resolution_clock::now();
         auto ns = std::chrono::duration_cast<std::chrono::nanoseconds> (t1-t0).count();
         std::cout << (flag ? "found in ":"not found in ") << ns << " ns" << std::endl;
    }
    return 0;
}
