#pragma once
#include <string>
#define ll long long
#define INT_MAX 2'147'483'647

enum type {
    INTEGER,
    STRING,
    DOUBLE,
    LONG
};

extern long int ARGS;
extern int CASES;
extern int MAX_STR_LEN;

void popInt     (ll n, int*&            arr);
void popLong    (ll n, ll*&             arr);
void popDouble  (ll n, double*&         arr);
void popString  (ll n, std::string*&    arr);