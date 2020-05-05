//
// Created by Tongbo Zhang.
//

#ifndef AMCX_BC_UTIL_H
#define AMCX_BC_UTIL_H


#include <iostream>
#include <vector>

class BenchmarkInfo {
public:
    std::string filename;
    int size;
    int realsize;
    BenchmarkInfo(std::string);
    std::vector<int> GetCRLits();
    bool PRCBenchmark();

private:
    std::string filename_ori; //original filename
    std::vector<int> Statistics; //begin with 1
    std::vector<std::pair<int, int>> CandidateLit;
    std::vector<int> CRCandidateLit;
    std::vector<int> BackBones;
    bool IsPreProcessed = false;
    bool IsLitCollected = false;
    bool IsXORCollected = false;
    bool IsCRLitGot = false;
    bool IsFinished = false;
    bool PreProcess();
    bool CollectLitInfo();
    bool CollectXORInfo();
    bool GetCRCandidateLit();
};

void DisplaySeparatorUp(std::string);

void DisplaySeparatorDown(std::string);

bool LitCompair(std::pair<int, int>, std::pair<int, int>);

double max_val(double, double);

double min_val(double, double);

#endif //AMCX_BC_UTIL_H
