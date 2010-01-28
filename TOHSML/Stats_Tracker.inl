/* Stats_Tracker.inl
 *
 * Author : Mitchell Keith Bloch, Soar Group at U-M
 * Date   : June/July 2008
 *
 * Implementation of Stats_Tracker.h
 */

#ifndef STATS_TRACKER_INL
#define STATS_TRACKER_INL

#include "Stats_Tracker.h"

#include <sml_Names.h>
#include <iostream>
#include <iomanip>

Stats_Tracker::Stats_Tracker()
: m_real_times("OS Real"),
m_proc_times("OS Proc"),
m_kernel_times("Soar Kernel"),
m_total_times("Soar Total")
{
}

void Stats_Tracker::time_run(sml::Agent &agent, const int &trial_number, const int &num_trials) {
  std::cout << std::endl << "***** Trial " << (trial_number + 1) << " of " << num_trials << " Begin *****" << std::endl;

  sml::ClientAnalyzedXML response0;
  sml::ClientAnalyzedXML response1;
  agent.ExecuteCommandLineXML("time run", &response0);
  agent.ExecuteCommandLineXML("stats", &response1);
  const double real_time = response0.GetArgFloat(sml::sml_Names::kParamRealSeconds, 0.0);
  const double proc_time = response0.GetArgFloat(sml::sml_Names::kParamProcSeconds, 0.0);
	const double kernel_time = response1.GetArgFloat(sml::sml_Names::kParamStatsKernelCPUTime, 0.0);
	const double total_time = response1.GetArgFloat(sml::sml_Names::kParamStatsTotalCPUTime, 0.0);
  add_times(real_time, proc_time, kernel_time, total_time);
  
  std::cout << std::endl << "***** Trial " << (trial_number + 1) << " of " << num_trials << " Complete *****" << std::endl;

  if(trial_number + 1 == num_trials)
    print_results();
}

void Stats_Tracker::clear() {
  m_real_times.clear();
  m_proc_times.clear();
  m_kernel_times.clear();
  m_total_times.clear();
}

void Stats_Tracker::add_times(const double &real_time, const double &proc_time,
                              const double &kernel_time, const double &total_time) {
  m_real_times.add_time(real_time);
  m_proc_times.add_time(proc_time);
  m_kernel_times.add_time(kernel_time);
  m_total_times.add_time(total_time);
}

void Stats_Tracker::print_results() const {
  std::cout << std::endl;
  std::cout << std::resetiosflags(std::ios::right) << std::setiosflags(std::ios::left);
	std::cout << std::setw(12) << " ";
	std::cout << " ";
	std::cout << std::resetiosflags(std::ios::left) << std::setiosflags(std::ios::right);
	std::cout << std::setw(10) << "Avg";
	std::cout << std::setw(10) << "Low";
  std::cout << std::setw(10) << "High" << std::endl;
  m_real_times.print_results();
  m_proc_times.print_results();
  m_kernel_times.print_results();
  m_total_times.print_results();
}

Stats_Tracker::Time_Stats::Time_Stats(const std::string &label)
: m_label(label),
time_count(0u),
low(-1.0),
high(-1.0),
average(-1.0)
{
}

void Stats_Tracker::Time_Stats::add_time(const double &time) {
  if(time_count) {
    const size_t time_count_next = time_count + 1;

    if(time < low)
      low = time;
    if(time > high)
      high = time;
    average = (average * time_count + time) / time_count_next;

    time_count = time_count_next;
  }
  else {
    time_count = 1u;
    low = time;
    high = time;
    average = time;
  }
}

void Stats_Tracker::Time_Stats::clear() {
  time_count = 0u;
  low = -1.0;
  high = -1.0;
  average = -1.0;
}

void Stats_Tracker::Time_Stats::print_results() const {
	std::cout.precision(3);
	std::cout << std::resetiosflags(std::ios::right) << std::setiosflags(std::ios::left);
	std::cout << std::setw(12) << m_label;
	std::cout << ":";
	std::cout << std::resetiosflags(std::ios::left);
	std::cout << std::setiosflags(std::ios::right);
	std::cout << std::setw(10) << std::setiosflags(std::ios::fixed) << std::setprecision(3) << average;
	std::cout << std::setw(10) << std::setiosflags(std::ios::fixed) << std::setprecision(3) << low;
	std::cout << std::setw(10) << std::setiosflags(std::ios::fixed) << std::setprecision(3) << high;
	std::cout << std::endl;
}

#endif
