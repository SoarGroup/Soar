/*
 * Stats_Tracker.h
 *
 *  Created on: Mar 13, 2017
 *      Author: mazzin
 */

#ifndef PERFORMANCETESTS_PERFORMANCETESTS_H_
#define PERFORMANCETESTS_PERFORMANCETESTS_H_

#include "portability.h"

#include <algorithm>
#include <assert.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <ctime>
#include <fstream>

#define QUIET_MODE
//#define BRIEF_MODE
#define DEFAULT_TRIALS 1
#define DEFAULT_DCS -1
#define DEFAULT_AGENT "count-test-5000";
#define DEFAULT_INITS 0

class StatsTracker
{
    public:
        std::vector<double> realtimes;
        std::vector<double> kerneltimes;
        std::vector<double> totaltimes;

        double GetAverage(std::vector<double> numbers)
        {
            assert(numbers.size() > 0 && "GetAverage: Size of set must be non-zero");

            double total = 0.0;
            for (unsigned int i = 0; i < numbers.size(); i++)
            {
                total += numbers[i];
            }
            return total / static_cast<double>(numbers.size());
        }

        double GetHigh(std::vector<double> numbers)
        {
            assert(numbers.size() > 0 && "GetHigh: Size of set must be non-zero");
            return *std::max_element(numbers.begin(), numbers.end());
        }

        double GetLow(std::vector<double> numbers)
        {
            assert(numbers.size() > 0 && "GetLow: Size of set must be non-zero");
            return *std::min_element(numbers.begin(), numbers.end());
        }

        void PrintResults(const char* testName)
        {
            std::ostringstream summary_string;

            //std::cout << std::endl << "Test Results:" << std::endl;
//            std::cout << std::endl;
//            std::cout << std::resetiosflags(std::ios::right) << std::setiosflags(std::ios::left);
//            std::cout << std::setw(12) << " ";
//            //std::cout << " ";
//            std::cout << std::resetiosflags(std::ios::left) << std::setiosflags(std::ios::right);
//            std::cout << std::setw(10) << "Avg";
//#ifndef BRIEF_MODE
//            std::cout << std::setw(10) << "Low";
//            std::cout << std::setw(10) << "High";
//            std::cout << std::setw(10) << "Spread" << std::endl;
//#else
//            std::cout << std::endl;
//#endif
            //PrintResultsHelper(std::cout, "OS Real", 12, GetAverage(realtimes), GetLow(realtimes), GetHigh(realtimes));
            PrintResultsHelper(std::cout, testName, 40, GetAverage(kerneltimes), GetLow(kerneltimes), GetHigh(kerneltimes), true);
            std::cout << "---------------------------------------------------------------------------------\n";
            //PrintResultsHelper(std::cout, "Soar Total", 12, GetAverage(totaltimes), GetLow(totaltimes), GetHigh(totaltimes));

            time_t t = time(0);
            struct tm * now = localtime( & t );
            std::ostringstream now_string;
            now_string << (now->tm_year + 1900) << '-' << (now->tm_mon + 1) << '-' <<  now->tm_mday << " " << now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec;
            summary_string << std::resetiosflags(std::ios::right) << std::setiosflags(std::ios::left);
            summary_string << std::setw(20) << now_string.str();
            summary_string << std::resetiosflags(std::ios::left);
            PrintResultsHelper(summary_string, testName, 40, GetAverage(kerneltimes), GetLow(kerneltimes), GetHigh(kerneltimes));

            std::ofstream resultFile("PerformanceTestResults.txt", std::ofstream::out | std::ofstream::app);
            resultFile << summary_string.str();
            resultFile.close();

            return;
        }

        void PrintResultsHelper(std::ostream &outStream, std::string label, int label_width, double avg, double low, double high, bool useColors = false)
        {
            outStream.precision(4);
            if (useColors) outStream << "\e[1;34m";
            outStream << std::resetiosflags(std::ios::right) << std::setiosflags(std::ios::left);
            outStream << std::setw(label_width) << label;
            if (useColors) outStream << "\e[0;37m";
            outStream << ":";
            if (useColors) outStream << "\e[0;93m";
            outStream << std::resetiosflags(std::ios::left);
            outStream << std::setiosflags(std::ios::right);
            outStream << std::setw(10) << std::setiosflags(std::ios::fixed) << std::setprecision(4) << avg;
#ifndef BRIEF_MODE
            outStream << std::setw(10) << std::setiosflags(std::ios::fixed) << std::setprecision(4) << low;
            outStream << std::setw(10) << std::setiosflags(std::ios::fixed) << std::setprecision(4) << high;
#endif
            outStream << std::setw(9) << std::setiosflags(std::ios::fixed) << std::setprecision(1) << (high ? ((high - low)/high)*100 : 0) << "%";
            if (useColors) outStream << "\e[0;37m";
            outStream << std::endl;
        }
};

#endif /* PERFORMANCETESTS_PERFORMANCETESTS_H_ */
