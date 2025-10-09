#pragma once
#include <string>
#define ll long long
#define INT_MAX 2'147'483'647
#define CLOCK chrono::high_resolution_clock::now()
#define DURATION chrono::duration<double>
#define ARGSLOOP for (ll i = 0; i < ARGS; ++i)
#define CASELOOP for (ll u = 0; u < CASES; ++u)

enum type {
    INTEGER,
    STRING,
    DOUBLE,
    LONG
};

extern long int ARGS;
extern int CASES;
extern int MAX_STR_LEN;
extern int testTypes;

void popInt         (ll n, int*&            arr);
void popLong        (ll n, ll*&             arr);
void popDouble      (ll n, double*&         arr);
void popString      (ll n, std::string*&    arr);