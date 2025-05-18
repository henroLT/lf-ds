#include <iostream>
#include <random>
#include <string>
#include <limits>
#include "tester.hpp"

using namespace std;

int CASES       = 100;
long int ARGS        = 10'000UL;
int MAX_STR_LEN = 10;

static mt19937_64 rng{random_device{}()};

void popInt(ll n, int* &arr) {
    arr = new int[n];

    static uniform_int_distribution<int> dist {
        numeric_limits<int>::min(),
        numeric_limits<int>::max()
    };

    for (ll i = 0; i < n; ++i) {
        arr[i] = dist(rng);
    }
}

void popLong(ll n, ll* &arr) {
    arr = new ll[n];

    static uniform_int_distribution<ll> dist {
        numeric_limits<ll>::min(),
        numeric_limits<ll>::max()
    };

    for(ll i = 0; i < n; ++i) {
        arr[i] = dist(rng);
    }
}

void popDouble(ll n, double* &arr) {
    arr = new double[n];

    static uniform_real_distribution<double> dist {
        numeric_limits<double>::lowest(),
        numeric_limits<double>::max()
    };

    for (ll i = 0; i < n; ++i) {
        arr[i] = dist(rng);
    }
}

void popString(ll n, string* &arr) {
    arr = new string[n];

    static const string charset = 
        "QWERTYUIOPASDFGHJKLZXCVBNM"
        "qwertyuiopasdfghjklzxcvbnm";
    static uniform_int_distribution<size_t> distC(0, charset.size() - 1);
    static uniform_int_distribution<int> distL(0, MAX_STR_LEN);

    for (ll i = 0; i < n; ++i) {
        string s;
        int len = distL(rng);
        s.reserve(len);
        for (int j = 0; j < len; j++) {
            s += charset[distC(rng)];
        }
        arr[i] = s;
    }
}

