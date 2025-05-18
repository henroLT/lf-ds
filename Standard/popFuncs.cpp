#include <iostream>
#include <random>
#include <string>
#include <limits>
#include "tester.hpp"

using namespace std;

int CASES       = 100;
long int ARGS   = 10'000UL;
int MAX_STR_LEN = 10;

static mt19937_64 rng{random_device{}()};
static uniform_int_distribution<int> distI {
        numeric_limits<int>::min(),
        numeric_limits<int>::max()
};
static uniform_int_distribution<ll> distL {
        numeric_limits<ll>::min(),
        numeric_limits<ll>::max()
};
static const string charset = 
        "QWERTYUIOPASDFGHJKLZXCVBNM"
        "qwertyuiopasdfghjklzxcvbnm";
static uniform_int_distribution<size_t> distC(0, charset.size() - 1);
static uniform_int_distribution<int> distLen(0, MAX_STR_LEN);


void popInt(ll n, int* &arr) {
    arr = new int[n];
    ARGSLOOP arr[i] = distI(rng);
}

void popLong(ll n, ll* &arr) {
    arr = new ll[n];
    ARGSLOOP arr[i] = distL(rng);
}

void popDouble(ll n, double* &arr) {
    arr = new double[n];
    ARGSLOOP arr[i] = (double) distI(rng) / (double) distI(rng);
}

void popString(ll n, string* &arr) {
    arr = new string[n];
    ARGSLOOP {
        string s;
        int len = distLen(rng);
        s.reserve(len);
        for (int j = 0; j < len; j++) s += charset[distC(rng)];
        arr[i] = s;
    }
}

