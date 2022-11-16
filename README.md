# Succint Data Structures Examples Using SDSL and C++

Compressed and succint data structures are able to save big amounts of memory while mantaining the ability to access and query efficiently. In this repo you will find some examples using [SDSL](https://github.com/simongog/sdsl-lite).

## Installing

To install the library please checkout [this](https://github.com/simongog/sdsl-lite). This library should probably not be used in production because is not mantained anymore. Checkout [this](https://github.com/vgteam/sdsl-lite) fork for a more updated version.

## Compressed integer vector

Integer usually occupy some fixed bytes of memory. I will assume 8 bytes from now on. Usual arrays store in memory the bytes one after the other. To read them you just have to skip 8 bytes per iteration, because each number is 8 bytes. What if we wanted to use less bits per integer? We'd have to implement a data structure that is able to store continuously some bits and that is able to read them one after the other. SDSL does this for us. But first, why would we ever do that? Is this even useful?

In many fields, like search engines, the web pages have an ID assigned to them and are stored in the so called inverted lists. Without going too much in detail, these are lists of inegers and are usually stored using gap coding, i.e. storing the difference between two consecutive integers instead of the integers themselves.

The first example uses `int_vector<5>` in `vector_example.cpp` to use only 5 bits per integer. This means that the maximum number we can represent is 31. Depending on the task, this simple but effective technique can reduce the memory occupation of a significant factor.

## Compressing using integer coders

Using a fixed amount of bits per integer may be a good idea in some particular cases but it has a big flaw that arises when the maximum number is very big and is an outlier compared to the other numbers. To solve this issue we can use variable length integer coders. In particular we use elias-$\gamma$ and elias-$\delta$ codes, developed by [Peter Elias](https://en.wikipedia.org/wiki/Peter_Elias). Why do we need them? Because otherwise we would not be able to recognize one integer from the other (remember that we are just storing a sequence of bits in memory).

### $\gamma$ code

$\gamma$ code represents each integer using the unary representation of its $length -1$ and the binary representation of the number. Let's understand this with an example.
The number 14 is 1110 in binary. These are 4 bits, so we prepend to it $3$ zeroes, which is the unary representation of the number $3$, obtaining 000 1110 (note that there would not be a space stored too, it's there just for clarity, the actual representation is 0001110) . Also note that every binary number starts with a 1, so the computer can be instructed to read some zeroes (and count them) until it finds a 1. Then it will read the following $numberOfZeroes+1$ bits to find out the actual number.
An issue of this coder is, you guessed it, the number of bits used. Let's say we need to represent a number that needs $10$ bits for its binary representation. We'll have to prepend $9$ zeros, what a waste of bits! $\delta$-code solves this.

### $\delta$ code

$\delta$ code fixes the above space issue by storing the unary representation of the length $-1$ of the binary represantation of the length of the number. Well, let's just make an example.
Let's say you want to store the number $14$ in $\delta$-code. The binary representation of it is 1110. The length of this is $4$, which is 110 in binary. The length of 110 is 3. The unary representation of $3-1 = 2$ is 00. Then we append the binary representation of the length of the number, which is 100 (the number $4$). Then we append the binary representation of the number itself, which is 1110 (the number $14$). The final result is 00 100 1110. I put some other examples in the table below, where **bn(n)** stands for "binary representation of n" and **u(n)** stands for "unary representation of n".
| n | bn(n) | len(bn(n)) |u(len(bn(n))-1)|$\delta$|
|--|--|--|--|--|
| 17 | 10001 | 5 = 101 |00| 00 101 10001|
| 32 | 100000 | 6 = 110 |00| 00 110 10000|
| 1024| 10000000000 | 11 = 1011 |000| 000 1011 10000000000 |

And that's it. Now we can compress any number using very few bits.

### Code for $\gamma$-code and $\delta$-code

SDSL implements $\gamma$-code using `sdsl::coder::elias_gamma` and $\delta$-code using `sdsl::coder::elias_delta`. Please check SDSL documentation for implementation details, given that it may be slightly different from what I described above. Check `integer_codes_examples.cpp` for some examples. To build the actual array you have to do the following

```c++
sdsl::int_vector<> v(10 * (1 << 20)); // a big array
v[100] = 1ULL << 63; // set the max
sdsl::util::bit_compress(v); // dynamically find the max and use the fixed bit number representation for v
sdsl::vlc_vector<sdsl::coder::elias_delta, 128> vlc(v); // compress using delta code. What is 128? Keep reading...
```

Why do we need that 128 in the code? That's the density. You basically store how many bits there are every 128 numbers. Let's make an example.
Suppose you choose to have density $=4$. This means that, in a separate array, you will store the "pointer" (the position) of every 4th number. If you have the array `{0, 0, 0, 0, 2, 3, 8, 0, 0}` you will obtain the bit vector `1 1 1 1 011 001000 0001001 1 1` (don't worry too much about the bit representation, know that SDSL uses a slightly different implementation compared to the one I described, for example it represents the number $0$ with the value $1$). Your other array, assuming density $=4$ will be `{0, 4, 22}`. Between position $0$ and position $4$ there are 4 numbers, which is our density. Between position $4$ and position $22$ there's 4 numbers. Why do I need this array? To access faster. Instead of scanning the whole bit vector SDSL will scan the support vector to find the number in position `i` in less time. Actually, it will probably do binary search on it, but again you should check implementation details for that.
In the file `integer_codes_examples.cpp` you'll also find the function `example2()` that shows how much a random array gets compressed. It' roughly a factor of 20 for $\delta$-code. That example is using gap coding on an increasing sequence (which happens very often in search engine programming), so if you use $\gamma$-code you'll actually get better compression because the gaps are small.

### Elias Fano representation

The biggest issue of representing integers with $\delta$-code is that you need a support data structure to random access the array. To solve this problem we can use the **Elias Fano** representation. SDSL gives us `sdsl::bit_vector` which is basically a specialization of `sdsl::int_vector<1>`. I will now explain Elias Fano assuming that we have the following two functions already implemented.

```c
select1(B, i){
	// B is a binary vector that contains 0s and 1s
	// i is the position
	// returns the position of the i-th bit set to 1 (counting from 1)
	// example: B = 1 0 0 1 0 1
	// calling select1(B, 2) returns 4, because the second 1 is in position 4
}
```

How can we implement the `select1` function? And how can we do it efficiently. I'm not going to explain this here, but there are ways to do this in constant time $O(1)$ with additional space equal to $o(n)$ (that's small o notation, so it takes less than $n$ bits). It's called `select1` because it counts the ones, we also have `select0`, the same exact function that works with `0`. There's also another function we need.

```c
rank(B, i){
	// returns the number of 1s from position 1 to position i
	// remember that the first position in the array is 1 and not 0
}
```

Again, this can be implemented to work in constant time. Now we can describe how the Elias Fano code works. This will require some math, nothing fancy. Elias Fano works on increasing integers, for example `S = {1, 4, 7, 18, 24, 26, 30, 31}`. We define `u = S[1] + S[-1] = 1 + 31 = 32`, `b = log(u) = 32` (this is $log_2(32)$), `l = log(u/n) = log 4 = 2`, where `n` is the length of `S`, and finally `h = b - l = 3`. We need all these variables to calculate the two arrays that actually encode the numbers. Let's first convert `S` to binary using `b` bits: `S = {00001, 00100, 00111, 10010, 11000, 11010, 11110, 11111}`. Now we define the two arrays `L` and `H`. We'll use `L` for the least significant bits of the numbers. We therefore take `l = 2` bit for each number, so we get `L = 01 00 11 10 00 10 10 11`. The first $2$ bits are `01` because the least significant bits of `00001` are `01`. The third and fourth bits are `00` because the $2$ least significant bits for `00100` are `00`, and so on. Now we create the `H` array, which contains information on the high part. We first notice that the `h = 3` more significant bits span from `000` to `111`, so they go from $0$ to $2^h - 1$. Elias Fano encodes `H` by counting how many of each configurations we have, starting from `0`. To make this clearer, let's crate a temporay array `T = 000 001 001 100 110 110 111 111` that contains all the first `h = 3` bits of each number. Now we just count the configurations and encode them in `H` in unary. We'll therefore have `H = 10 110 0 0 10 0 110 110`. Let's check this. How many configurations `000` do we have? Just one, therefore we write `1` followed by a `0`, which acts as a separator (otherwise we would not be able to understand where to stop counting for the configuration `000`). Now we check the following configuration, which is `001` and we see that we have $2$ of them, therefore we write `110`, the unary code of $2$. Then we have the configuration `010`, which never happens so we just put a `0`, and so on.

Now we just need a way to `access` (that's the name of the function too) a number at a specific index. We assume that we have the `select1` and `rank` implemented. The following is the implementation of `access`.

```c
access(L, H, i){
	low = L[(i-1)*l + 1 : i*l] // python-like notation
	high = to_binary(select1(H, i) - i)
	return low + high // + operator to appen high to low
}
```

Remember that `i` starts from $1$ (I won't repeat this anymore, but I think that it's useful to repeat it because computer scientists are used to arrays starting from `0`). To understand the above code let's make an example. Let's say we want to execute `access(5)`. To get the low part we have to get the low part of the fifth number which is `00` from `L = 01 00 11 10 00 10 10 11`, in position `9` and `10`. For the high part, you need to count the number of ones before position `i`, using the select function. Now we subtract `i` from it and obtain the number of zeroes. So we have `11 - 5 = 6 = 110`. What does the number of zeroes represent? It's the number of binary configurations! So when we get `6` we know that it actually is the most significant part of the number we're searching for. Our result it therefore 11000, which is $24$, the fifth number.

There's also another very interesting function in Elias Fano called `nextGEQ(n)`. This returns the number in the array greater or equal than `n`, and it does so very fast (constant time).

### Elias Fano Code

You will not find any complete SDSL code examples in this repo, but I'll give you some guidelines. First of all, as I already said, you will use `sdsl::bit_vector` which is just a specalization of `sdsl::int_vector<1>`. For example you can write:

```c
sdsl::bit_vector b = {0,1,0,1,1};
// or
sdsl::bit_vector b1(100, 0); // creates a vector with 100 bits all set to 0
```

Now you need the data structures to support `rank` and `select`. We don't know how these work internally, but someone already implemented them for us. Use the following code.

```c
sdsl::bit_vector b = {0,1,0,1,1};
sdsl::rank_support_v<1> b_rank(&b); // to obtain a data structure that supports the rank() operation through b_rank(i)
sdsl::select_support_mlc<1> b_select(&b); // to obtain a data structure that supports select(i) through b_select(i)
```

Now we compress through Elias Fano in the following way.

```c
sdsl::sd_vector<> ef(b); // elias fano of b
// L is contained in ef.low
// H is contained in ef.high
```

Know that there's a known issue (bug) that will give you different results compared to the ones i described. If you use [other](https://github.com/vgteam/sdsl-lite) libraries this is fixed.

## Pointerless programming

Our last example is pointerless programming, to work with strings. As you probably understood by now, when working with big data we want to save every bit. Very simple problems are unfeasible when you have giga bytes, tera bytes or even peta bytes of data. We'll solve a very simple problem with string, trying to be as efficient as possible. We want to store strings in alphabetical order and support searches. What we could do is a `char** v;`an array of pointers to `char*`, but this approach uses `64 bits` for each pointer (ie for each string) and requires you to jump in memory to get the string, possibly causing a cache miss. Instead of doing this, we'll use the `sdsl::bit_vector` to speed things up. Take a look at the following example, remembering that the special character `\0` is the string terminator character.

```c
s = {"dog", "cat", "parrot"}
// store the strings 1 after the other using just a char*v
// dog\0cat\0parrot\0
// 1000 1000 1000000 // use this bit array to indicate where each string starts
```

For implementation details checkout `pointerless_programming.cpp`. The idea is to store the strings and the `\0` in just a `char*v` and to keep another bit vector where we store `1` where a string starts and then as many zeroes as the length of the string `-1` plus another `0` for the string terminator. By doing this we don't store any pointer and we don't have cache misses. You can then compress the bit array with Elias Fano.
To search for a number we do a binary search in the bit array.
