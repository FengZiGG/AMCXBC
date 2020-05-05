//
// Created by Tongbo Zhang.
//

#include <iomanip>
#include <string>
#include <algorithm>
#include "util.h"

BenchmarkInfo::BenchmarkInfo(std::string filename) {
    this->filename = filename;
    filename_ori = filename;
}

bool BenchmarkInfo::PreProcess() {
    if (IsPreProcessed)
        return false;
    DisplaySeparatorUp("PreProcessing");
#ifdef __APPLE__
    std::string cmd =
            "PreProc_mac -vivification -eliminateLit -litImplied -iterate=100 -equiv -orGate -affine " + filename +
            " > " +
            filename + "_pp" + " 2> " + filename + "_cerr";
#endif

#ifdef __linux__
    std::string cmd =
        "PreProc_linux -vivification -eliminateLit -litImplied -iterate=10 -equiv -orGate -affine " + filename + " > " +
        filename + "_pp" + " 2> " + filename + "_cerr";
#endif
    FILE *fp = popen(cmd.c_str(), "r");
    pclose(fp);
    IsPreProcessed = true;
    filename = filename_ori + "_pp";
    return true;
}

bool BenchmarkInfo::CollectLitInfo() {
    if (!IsPreProcessed || IsLitCollected)
        return false;
    DisplaySeparatorUp("CollectLitInfo");
    FILE *fp;
    char buffer[1024];
    int VarNum;
    int ClauseNum;
    int lit = -1;

    fp = fopen(filename.c_str(), "r");

    if (fp == NULL) {
        std::cerr << "Cannot find the input file. Make sure the filename is correct." << std::endl;
        exit(0);
    }

    fgets(buffer, 1024, fp);
    while (buffer[0] != 'p') {
        fgets(buffer, 1024, fp);
    }

    sscanf(buffer, "p cnf %d %d", &VarNum, &ClauseNum);

    Statistics.assign(VarNum + 1, 0);

    for (int i = 0; i < ClauseNum; i++) {
        bool FirstTime = true;
        int FirstLit;
        fscanf(fp, "%d", &lit);
        lit = abs(lit);
        FirstLit = lit;
        while (lit != 0) {
            Statistics[lit]++;
            fscanf(fp, "%d", &lit);
            lit = abs(lit);
            if (FirstTime && lit == 0)
                Statistics[FirstLit] = -1;
            else {
                FirstTime = false;
            }
        }
    }
    fclose(fp);
    IsLitCollected = true;
    return true;
}

bool BenchmarkInfo::CollectXORInfo() {
    if (!IsLitCollected || IsXORCollected)
        return false;
    DisplaySeparatorUp("CollectXORInfo");
    for (int i = 1; i < Statistics.size(); i++) {
        if (Statistics[i] != -1)
            CandidateLit.push_back(std::make_pair(i, Statistics[i]));
        else
            BackBones.push_back(i);
    }
    sort(CandidateLit.begin(), CandidateLit.end(), LitCompair);
    std::cerr << std::left << std::setw(20) << "BackBoneSize:" << BackBones.size() << std::endl;
    std::cerr << std::left << std::setw(20) << "CandidateXORSize:" << CandidateLit.size() << std::endl;
    IsXORCollected = true;
    return true;
}

bool BenchmarkInfo::GetCRCandidateLit() {
    if (!IsXORCollected || IsCRLitGot)
        return false;
    DisplaySeparatorUp("GetCRCandidateLit");
    int sum = 0;
    double maxnum = 0;
    for (int i = 0; i < CandidateLit.size(); i++) {
        maxnum = maxnum > CandidateLit[i].second ? maxnum : CandidateLit[i].second;
    }

    for (int i = 0; i < CandidateLit.size(); i++) {
        sum += int(double(maxnum - CandidateLit[i].second) / maxnum * 2 + 1);
    }
    for (int i = 0; i < CandidateLit.size(); i++) {
        for (int j = 0; j < int(double(maxnum - CandidateLit[i].second) / maxnum * 2 + 1); j++) {
            CRCandidateLit.push_back(CandidateLit[i].first);
        }
    }
    std::cerr << std::left << std::setw(20) << "CRCandidateLitSize:" << CRCandidateLit.size() << std::endl;
    IsCRLitGot = true;
    return true;
}

bool BenchmarkInfo::PRCBenchmark() {
    PreProcess();
    CollectLitInfo();
    CollectXORInfo();
    GetCRCandidateLit();
    if (IsCRLitGot)
        IsFinished = true;
    size = CRCandidateLit.size();
    realsize = CandidateLit.size();
    return IsFinished;
}

std::vector<int> BenchmarkInfo::GetCRLits() {
    if (!IsFinished)
        PRCBenchmark();
    return CRCandidateLit;
}

/*
void DisplayLitStatistics() {
    DisplaySeparatorUp("Display LitStatistics");
    std::cerr << "┌───────┬───────┐" << std::endl;
    std::cerr << std::left << std::setw(10) << "│VARNUM" << "│" << std::right << std::setw(10) << "TIMES│" << std::endl;
    for (int i = 1; i < LitStatistics.size(); i++) {
        std::cerr << "│" << std::left << std::setw(7) << i << "│" << std::right << std::setw(7) << LitStatistics[i] << "│" << std::endl;
    }
    std::cerr << "└───────┴───────┘" << std::endl;
    DisplaySeparatorDown("Display Down");
}

void DisplayLitCandidate() {
    DisplaySeparatorUp("Display LitCandidate");
    std::cerr << "┌───────┬───────┐" << std::endl;
    std::cerr << std::left << std::setw(10) << "│VARNUM" << "│" << std::right << std::setw(10) << "TIMES│" << std::endl;
    for (int i = 1; i < LitCandidateXOR.size(); i++) {
        std::cerr << "│" << std::left << std::setw(7) << LitCandidateXOR[i].first << "│" << std::right << std::setw(7)
             << LitCandidateXOR[i].second << "│" << std::endl;
    }
    std::cerr << "└───────┴───────┘" << std::endl;
    DisplaySeparatorDown("Display Down");
}
*/

bool LitCompair(std::pair<int, int> p1, std::pair<int, int> p2) {
    if (p1.second > p2.second)
        return true;
    else
        return false;
}

void DisplaySeparatorUp(std::string info = "") {
    std::cerr << "┌───────────────────────────────────────────────────────────" << std::endl;
    if (info != "")
        std::cerr << "│" << info << std::endl;
    std::cerr << "└───────────────────────────────────────────────────────────" << std::endl;
}

void DisplaySeparatorDown(std::string info = "") {
    std::cerr << "┌───────────────────────────────────────────────────────────" << std::endl;
    if (info != "")
        std::cerr << "│" << info << std::endl;
    std::cerr << "└───────────────────────────────────────────────────────────" << std::endl;
}

double max_val(double a, double b) {
    return a > b ? a : b;
}

double min_val(double a, double b) {
    return a < b ? a : b;
}
