#include <iostream>
#include <chrono>
#include <queue>
#include <vector>
#include "tester.hpp"

using namespace std;

extern int CASES;
extern long int ARGS;
extern int MAX_STR_LEN;
constexpr type TYPE = type::INTEGER;

template <type T>
using selected = conditional_t<
    T == type::INTEGER, int,
    conditional_t<
        T == type::STRING, string,
        conditional_t<
            T == type::DOUBLE, double,
                long long
        >
    >
>;

template <typename T>
class tester {
    private:
        queue<T> tester;
        T* population;

    public:
        void populate() {
            if constexpr (TYPE == type::INTEGER) popInt(ARGS, population);
            else if constexpr (TYPE == type::DOUBLE) popDouble(ARGS, population);
            else if constexpr (TYPE == type::LONG) popLong(ARGS, population);
            else if constexpr (TYPE == type::STRING) popString(ARGS, population);
            /* add more cases as needed */
        }

        double pushT() {

        }

        double popT() {

        }

        void spit() {
            if constexpr (TYPE == type::INTEGER) {
                for (int i = 0; i < ARGS; ++i) printf("%d ", population[i]);
            }
            else if constexpr (TYPE == type::DOUBLE) {
                for (int i = 0; i < ARGS; ++i) printf("%f ", population[i]);
            }
            else if constexpr (TYPE == type::LONG) {
                for (int i = 0; i < ARGS; ++i) printf("%ld ", population[i]);
            }
            else if constexpr (TYPE == type::STRING) {
                for (int i = 0; i < ARGS; ++i) printf("%s ", population[i]);
            }
            printf("\n");
        }

        void clean() {
            delete[] population;
        }
};


int main() {
    tester<selected<TYPE>> driver;


    driver.populate();
    driver.spit();
    driver.clean();
    

    return 0;
}