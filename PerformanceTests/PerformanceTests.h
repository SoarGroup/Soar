/*
 * Stats_Tracker.h
 *
 *  Created on: Mar 13, 2017
 *      Author: mazzin
 */

#ifndef PERFORMANCETESTS_PERFORMANCETESTS_H_
#define PERFORMANCETESTS_PERFORMANCETESTS_H_

#include "portability.h"

#include <assert.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#define QUIET_MODE
#define BRIEF_MODE
#define DEFAULT_TRIALS 3
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

            double high = numbers[0];
            for (unsigned int i = 0; i < numbers.size(); i++)
            {
                if (numbers[i] > high)
                {
                    high = numbers[i];
                }
            }
            return high;
        }

        double GetLow(std::vector<double> numbers)
        {
            assert(numbers.size() > 0 && "GetLow: Size of set must be non-zero");

            double low = numbers[0];
            for (unsigned int i = 0; i < numbers.size(); i++)
            {
                if (numbers[i] < low)
                {
                    low = numbers[i];
                }
            }
            return low;
        }

        void PrintResults()
        {
            std::cout << std::endl << "Test Results:" << std::endl;
            std::cout << std::resetiosflags(std::ios::right) << std::setiosflags(std::ios::left);
            std::cout << std::setw(12) << " ";
            std::cout << " ";
            std::cout << std::resetiosflags(std::ios::left) << std::setiosflags(std::ios::right);
            std::cout << std::setw(10) << "Avg";
#ifndef BRIEF_MODE
            std::cout << std::setw(10) << "Low";
            std::cout << std::setw(10) << "High" << std::endl;
#else
            std::cout << std::endl;
#endif
            PrintResultsHelper("OS Real", GetAverage(realtimes), GetLow(realtimes), GetHigh(realtimes));
            PrintResultsHelper("Soar Kernel", GetAverage(kerneltimes), GetLow(kerneltimes), GetHigh(kerneltimes));
            PrintResultsHelper("Soar Total", GetAverage(totaltimes), GetLow(totaltimes), GetHigh(totaltimes));
        }

        void PrintResultsHelper(std::string label, double avg, double low, double high)
        {
            std::cout.precision(3);
            std::cout << std::resetiosflags(std::ios::right) << std::setiosflags(std::ios::left);
            std::cout << std::setw(12) << label;
            std::cout << ":";
            std::cout << std::resetiosflags(std::ios::left);
            std::cout << std::setiosflags(std::ios::right);
            std::cout << std::setw(10) << std::setiosflags(std::ios::fixed) << std::setprecision(3) << avg;
#ifndef BRIEF_MODE
            std::cout << std::setw(10) << std::setiosflags(std::ios::fixed) << std::setprecision(3) << low;
            std::cout << std::setw(10) << std::setiosflags(std::ios::fixed) << std::setprecision(3) << high;
#endif
            std::cout << std::endl;
        }
};

#endif /* PERFORMANCETESTS_PERFORMANCETESTS_H_ */
