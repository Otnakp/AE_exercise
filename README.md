# Succint Data Structures Examples Using SDSL and C++

Compressed and succint data structures are able to save big amounts of memory while mantaining the ability to access and query efficiently. In this repo you will find some examples using [SDSL](https://github.com/simongog/sdsl-lite).

## Compressed integer vector

Integer usually occupy some fixed bytes of memory. I will assume 8 bytes from now on. Usual arrays store in memory the bytes one after the other. To read them you just have to skip 8 bytes per iteration, because each number is 8 bytes. What if we wanted to use less bits per integer? We'd have to implement a data structure that is able to store continuously some bits and that is able to read them one after the other. SDSL does this for us. But first, why would we ever do that? Is this even useful?

In many fields, like search engines, the web pages have an ID assigned to them and are stored in the so called inverted lists. Without going too much in detail, these are lists of inegers and are usually stored using gap coding, i.e. storing the difference between two consecutive integers instead of the integers themselves.

The first example uses `int_vector<5>` in `vector_example.cpp` to use only 5 bits per integer. This means that the maximum number we can represent is 31. Depending on the task, this simple but effective technique can reduce the memory occupation of a significant factor.

## Compressing using integer coders

Using a fixed amount of bits per integer may be a good idea in some particular cases but it has a big flaw that arises when the maximum number is very big and is an outlier compared to the other numbers. To solve this issue we can use variable length integer coders. In particular we use elias-$\gamma$ and elias-$\delta$ codes, developed by [Peter Elias](https://en.wikipedia.org/wiki/Peter_Elias). Why do we need them? Because otherwise we would not be able to recognize one integer from the other (remember that we are just storing a sequence of bits in memory).

## $\gamma$ code

$\gamma$ code represents each integer using the unary representation of its $length -1$ and the binary representation of the number. Let's understand this with an example.
The number 14 is 1110 in binary. These are 4 bits, so we prepend to it $3$ zeroes, which is the unary representation of the number $3$, obtaining 000 1110 (note that there would not be a space stored too, it's there just for clarity, the actual representation is 0001110) .
An issue of this coder is, you guessed it, the number of bits used. Let's say we need to represent a number that needs $10$ bits for its binary representation. We'll have to prepend $9$ zeros, what a waste of bits! $\delta$-code solves this.

## $\delta$ code

$\delta$ code fixes the above space issue by storing the unary representation of the length $-1$ of the binary represantation of the length of the number. Well, let's just make an example.
Let's say you want to store the number $14$ in $\delta$-code. The binary representation of it is 1110. The length of this is $4$, which is 110 in binary. The length of 110 is 3. The unary representation of $3-1 = 2$ is 00. Then we append the binary representation of the length of the number, which is 100 (the number $4$). Then we append the binary representation of the number itself, which is 1110 (the number $14$). The final result is 00 100 1110. I put some other examples in the table below, where **bn(n)** stands for "binary representation of n" and **u(n)** stands for "unary representation of n".
| n | bn(n) | len(bn(n)) |u(len(bn(n))-1)|$\delta$|
|--|--|--|--|--|
| 17 | 10001 | 5 = 101 |00| 00 101 10001|
| 32 | 100000 | 6 = 110 |00| 00 110 10000|
| 1024| 10000000000 | 11 = 1011 |000| 000 1011 10000000000 |
And that's it. Now we can compress any number using very few bits.

## Code for $\gamma$-code and $\delta$-code

SDSL implements $\gamma$-code using `sdsl::coder::elias_gamma` and $\delta$-code using `sdsl::coder::elias_delta`. Please check SDSL documentation for implementation details, given that it may be slightly different from what I described above. Check `integer_codes_examples.cpp` for some examples. To build the actual array you have to do the following
`sdsl::int_vector<> v(10 * (1 << 20)); // a big array v[100] = 1ULL << 63; // set the max sdsl::util::bit_compress(v); // dynamically find the max and use the fixed bit number representation for v sdsl::vlc_vector<sdsl::coder::elias_delta, 128> vlc(v); // compress using delta code. What is 128? Keep reading...`
Why do we need that 128 in the code? That's the density. You basically store how many bits there are every 128 numbers. Let's make an example.
Suppose you choose to have density $=4$. This means that, in a separate array, you will store the "pointer" (the position) of every 4th number. If you have the array `{0, 0, 0, 0, 2, 3, 8, 0, 0}` you will obtain the bit vector `1 1 1 1 011 001000 0001001 1 1` (don't worry too much about the bit representation, know that SDSL uses a slightly different implementation compared to the one I described, for example it represents the number $0$ with the value $1$). Your other array, assuming density $=4$ will be `{0, 4, 22}`. Between position $0$ and position $4$ there are 4 numbers, which is our density. Between position $4$ and position $22$ there's 4 numbers. Why do I need this array? To access faster. Instead of scanning the whole bit vector SDSL will scan the support vector to find the number in position `i` in less time. Actually, it will probably do binary search on it, but again you should check implementation details for that.
In the file `integer_codes_examples.cpp` you'll also find the function `example2()` that shows how much a random array gets compressed. It' roughly a factor of 20 for $\delta$-code. That example is using gap coding on an increasing sequence (which happens very often in search engine programming), so if you use $\gamma$-code you'll actually get better compression because the gaps are small.
