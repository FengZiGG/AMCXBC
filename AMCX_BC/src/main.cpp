//
// Created by Tongbo Zhang.
//

#include <cryptominisat5/dimacsparser.h>
#include <boost/math/distributions/normal.hpp>
#include <algorithm>
#include <set>
#include <sys/time.h>
#include "solverconf.h"
#include "util.h"

using namespace std;
using namespace CMSat;

unsigned int gen_random(unsigned int n) {
    return rand() % n;
}

bool check_sat(SATSolver &s, unsigned int &times) {
    times++;
    if (s.solve() == CMSat::l_True) {
        return true;
    } else if (s.solve() == CMSat::l_False) {
        return false;
    } else {
        cout << "error: Sat solver says unknown." << endl;
        exit(1);
    }
}

inline bool count_sat(SATSolver &s, unsigned int &times, unsigned int &sharpF) {
    times++;
    s.set_allow_otf_gauss();
    int max_model_counter = 512;
    if (s.solve() == l_True) {
        int sum = 0;
        while (s.solve() == l_True) {
            vector<Lit> ban_solution;
            for (uint32_t var = 0; var < s.nVars(); var++) {
                if (s.get_model()[var] != l_Undef) {
                    ban_solution.push_back(Lit(var, s.get_model()[var] == l_True));
                }
            }
            s.add_clause(ban_solution);
            sum++;
            if (sum > max_model_counter) {
                return true;
            }
        }
        sharpF = sum;
        return false;
    }
    sharpF = 0;
    return false;

}

void gen_hash_func(SATSolver &s, std::vector<unsigned int> a, BenchmarkInfo &benchmarkInfo) {
    vector<int> CRLits = benchmarkInfo.GetCRLits();
    vector<unsigned int> lits;
    set<unsigned int> setlits;

    for (int i = 0; i < a.size() - 1; i++)
        setlits.insert(CRLits[a[i]] - 1);
    for (auto i:setlits) {
        lits.push_back(i);
    }

    bool flag;

    if (a.back() == 0)
        flag = false;
    else
        flag = true;

    s.add_xor_clause(lits, flag);
}

void gen_hash_func_1(SATSolver &s, std::vector<bool> a, BenchmarkInfo &benchmarkInfo) {
    vector<int> CRLits = benchmarkInfo.GetCRLits();
    assert(a.size() == CRLits.size() + 1);
    vector<unsigned int> lits;
    set<unsigned int> setlits;
    for (unsigned int i = 0; i < CRLits.size(); i++) {
        if (a[i])
            setlits.insert(CRLits[i] - 1);
    }
    for (auto i:setlits)
        lits.push_back(i);
    s.add_xor_clause(lits, a.back());
}

void gen_hash_func_2(SATSolver &s, std::vector<bool> a) {
    assert(a.size() == s.nVars() + 1);
    vector<unsigned int> lits;
    for (unsigned int i = 0; i < s.nVars(); i++) {
        if (a[i])
            lits.push_back(i);
    }
    cout << "xor size: " << lits.size() << endl;
    for (auto i:lits)
        cout << i << " ";
    cout << endl;
    s.add_xor_clause(lits, a.back());
}

std::vector<unsigned int> gen_random_param(unsigned int size, unsigned int randomsize) {
    std::vector<unsigned int> vec;
    for (unsigned int i = 0; i < size; i++)
        vec.push_back(gen_random(randomsize));
    vec.push_back(gen_random(2));
    return vec;
}

std::vector<bool> gen_random_param_1(unsigned int size) {
    std::vector<bool> vec;
    for (unsigned int i = 0; i < size; i++) {
        if (gen_random(2) == 0) {
            vec.push_back(false);
        } else {
            vec.push_back(true);
        }
    }
    return vec;
}

void read_file(SATSolver &s, string filename, SolverConf &conf) {
#ifndef USE_ZLIB
    FILE *in = fopen(filename.c_str(), "r");
#else
    gzFile in = gzopen(filename, "rb"
#endif

#ifndef USE_ZLIB
    DimacsParser<StreamBuffer<FILE *, FN> > parser(&s, NULL, conf.verbosity);
#else
    DimacsParser<StreamBuffer<gzFile, GZ> > parser(solver, NULL, conf.verbosity);
#endif

    if (!parser.parse_DIMACS(in, false)) {
        exit(-1);
    }

#ifndef USE_ZLIB
    fclose(in);
#else
    gzclose(in);
#endif

}

int main(int argc, char **argv) {
    srand((int) time(0));
    if (argc != 3) {
        std::cout << "USAGE: ./AMCX_RPC <delta> <input-file>" << std::endl;
        exit(1);
    }
    SolverConf conf;
    const long double delta = atof(argv[1]);
    string filename = argv[2];
    timeval start, preproctime, end;
    gettimeofday(&start, nullptr);

    BenchmarkInfo benchmarkInfo(filename);
    benchmarkInfo.PRCBenchmark();
    gettimeofday(&preproctime, nullptr);

    std::vector<unsigned int> C;
    vector<long double> L;
    vector<long double> R;
    unsigned int t = 0;
    unsigned int query_counter = 0;
    bool firsttime = true;
    int roughavg = 0;
    SATSolver roughsolver;
    roughsolver.set_allow_otf_gauss();
    //roughsolver.set_num_threads(6);
    read_file(roughsolver, filename, conf);

    DisplaySeparatorUp("RoughAVG");
    while (check_sat(roughsolver, query_counter)) {
        gen_hash_func(roughsolver, gen_random_param(benchmarkInfo.realsize, benchmarkInfo.size), benchmarkInfo);
        roughavg++;
        cout << roughavg << endl;
    }

    while (true) {
        t++;
        vector<vector<unsigned int>> matA;
        long double depth = 0;
        //set start depth
        long double avg = 0;

        if (firsttime) {
            avg = roughavg;
            firsttime = false;
        } else {
            for (unsigned int i = 0; i < L.size(); i++)
                avg += L[i];
            avg /= L.size();
        }
        unsigned int offset = 5;
        for (unsigned int i = 0; i < avg - offset; i++) {
            matA.push_back(gen_random_param(benchmarkInfo.realsize, benchmarkInfo.size));
            depth++;
        }

        unsigned int sharpF = -1;
        while (true) {
            SATSolver counter;
            read_file(counter, filename, conf);
            for (unsigned int i = 0; i < depth; i++) {
                gen_hash_func(counter, matA[i], benchmarkInfo);
            }
            matA.push_back(gen_random_param(benchmarkInfo.realsize, benchmarkInfo.size));
            gen_hash_func(counter, matA.back(), benchmarkInfo);
            depth++;
            if (!count_sat(counter, query_counter, sharpF)) {
                depth = depth + log(sharpF) / log(2);
                break;
            }
        }

        L.push_back(depth);
        stringstream stream;
        stream << "Iteration: " << t << "\tDepth: " << depth << "\tTemp: " << powl(2, depth);
        DisplaySeparatorUp(stream.str());
        R.push_back(powl(2, depth));

        if (t > 35 * log(3 / delta) / log(2)) {
            gettimeofday(&end, nullptr);
            sort(R.begin(), R.end());
            cout << fixed;
            long double middle = 0;
            if (R.size() % 2 == 1)
                middle = R[floor((long double) (R.size()) / 2.0)];
            else
                middle = (long double) ((R[R.size() / 2] + R[R.size() / 2 - 1])) / 2.0;
            cout << "Result_ORI= " << middle << endl;
            cout.unsetf(ios_base::fixed);
            cout << setprecision(4) << "Result_SCI= " << middle << endl;

            double time = 1000.0 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000.0;
            time = time / 1000.0;
            cout << "Time(s): " << time << endl;
            time = 1000.0 * (preproctime.tv_sec - start.tv_sec) + (preproctime.tv_usec - start.tv_usec) / 1000.0;
            time = time / 1000.0;
            cout << "Time_PreProc(s): " << time << endl;
            break;
        }
    }
    return 0;
}
