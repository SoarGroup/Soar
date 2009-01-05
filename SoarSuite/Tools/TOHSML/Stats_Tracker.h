/* Stats_Tracker.h
 *
 * Author : Mitchell Keith Bloch, Soar Group at U-M
 * Date   : June/July 2008
 *
 * Simple stats tracker taken from TestSoarPerformance.cpp 
 * and modified.
 */

#ifndef STATS_TRACKER_H
#define STATS_TRACKER_H

#include "sml_Client.h"
#include <vector>
#include <string>

class Stats_Tracker {
public:
  inline Stats_Tracker();

  inline void time_run(sml::Agent &agent, const int &trial_number, const int &num_trials);

  inline void clear();

private:

  inline void add_times(const double &real_time, const double &proc_time,
                        const double &kernel_time, const double &total_time);

	inline void print_results() const;

  struct Time_Stats {
    inline Time_Stats(const std::string &label);

    inline void add_time(const double &time);
    inline void clear();
    inline void print_results() const;

    std::string m_label;
    size_t time_count;
    double low;
    double high;
    double average;
  };

  Time_Stats m_real_times;
  Time_Stats m_proc_times;
  Time_Stats m_kernel_times;
  Time_Stats m_total_times;
};

#endif
