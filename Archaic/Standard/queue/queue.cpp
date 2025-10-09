#include <iostream>
#include <chrono>
#include <queue>
#include "tester.hpp"

using namespace std;

extern int CASES;
extern long int ARGS;
constexpr type TYPE = type::LONG;

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

        double overheadPop() {
            auto start = CLOCK;
            ARGSLOOP;
            auto end = CLOCK;

            DURATION dur = (end - start);
            return dur.count();
        }

        double overheadPush() {
            auto start = CLOCK;
            ARGSLOOP population[i];
            auto end = CLOCK;

            DURATION dur = (end - start);
            return dur.count();
        }

    public:

        void populate() {
            if      constexpr (TYPE == type::INTEGER)   popInt(ARGS, population);
            else if constexpr (TYPE == type::DOUBLE)    popDouble(ARGS, population);
            else if constexpr (TYPE == type::LONG)      popLong(ARGS, population);
            else if constexpr (TYPE == type::STRING)    popString(ARGS, population);
            /* add more cases as needed */
        }

        double pushT() {
            auto start = CLOCK;
            ARGSLOOP tester.push(population[i]);
            auto end = CLOCK;

            DURATION dur = (end - start);
            return dur.count() - this->overheadPush();
        }

        double popT() {
            auto start = CLOCK;
            ARGSLOOP tester.pop();
            auto end = CLOCK;

            DURATION dur = (end - start);
            return dur.count() - this->overheadPop();
        }


        void spit() {
            if      constexpr (TYPE == type::INTEGER)   ARGSLOOP printf("%d ", population[i]);
            else if constexpr (TYPE == type::DOUBLE)    ARGSLOOP printf("%lf ", population[i]);
            else if constexpr (TYPE == type::LONG)      ARGSLOOP printf("%lld ", population[i]);
            else if constexpr (TYPE == type::STRING)    ARGSLOOP printf("%s ", population[i].c_str());
        }

        void clean() {
            delete[] population;
        }
};


string TYPEtoString() {
    switch(TYPE) {
        case INTEGER:   return "INTEGER";
        case DOUBLE:    return "DOUBLE";
        case LONG:      return "LONG";
        case STRING:    return "STRING";
        /* add cases as needed */
    }

    return NULL;
}

int main() {
    tester<selected<TYPE>> driver;
    double avgPOP = 0.0, avgPUSH = 0.0;

    CASELOOP {
        driver.populate();
        avgPUSH += driver.pushT();
        avgPOP += driver.popT();
    }

    printf("Using TYPE: %s\n", TYPEtoString().c_str());
    printf("Average PUSH time over CASES: %d with ARGS: %ld, is: %.10f\n", 
            CASES, ARGS, avgPUSH / CASES);
    printf("Average POP time over CASES: %d with ARGS: %ld, is: %.10f\n", 
            CASES, ARGS,  avgPOP / CASES);

    driver.clean();
    return 0;
}