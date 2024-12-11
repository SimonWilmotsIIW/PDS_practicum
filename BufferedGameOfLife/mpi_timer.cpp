#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <iostream>
#include <cmath>
#include <mpi.h>

class Timer {
public:
    Timer(bool autoStart = true) {
		if (autoStart)
			start();
    }

    void start() {
        beg = MPI_Wtime();
    }

    void stop() {
        end = MPI_Wtime();
    }

    double durationSeconds() {
        return end - beg;
    }
private:
    double beg;
    double end;
};

class AutoAverageTimer : public Timer
{
public:
	AutoAverageTimer(const std::string &name) : Timer(false), m_name(name), m_justReported(false) { }
	~AutoAverageTimer()
	{
		if (!m_justReported)
			report();
	}

	void stop()
	{
		Timer::stop();
		m_times.push_back(durationSeconds());
		m_justReported = false;
	}

	void report(std::ostream &o = std::cout, double multiplier = 1, const std::string suffix = " sec")
	{
		double avg = 0;
		for (auto x : m_times){
			// std::cout << x * multiplier << " ";
			avg += x;
		}
		std::cout << std::endl;
		if (m_times.size() > 0)
			avg /= m_times.size();
		double stddev = 0;
		for (auto x : m_times)
		{
			double dt = x - avg;
			stddev += dt*dt;
		}
		if (m_times.size() > 0)
			stddev /= m_times.size();
		stddev = std::sqrt(stddev);
		//printf("# %s %f +/- %f%s (%ld measurements)\n", m_name.c_str(), avg*multiplier, stddev*multiplier, suffix.c_str(), m_times.size());
		o << "#" << m_name << " " << avg*multiplier << " +/- " << stddev*multiplier << suffix << " (" << m_times.size() << " measurements)" << std::endl;
		m_justReported = true;
	}

	void reset(){
		m_times.clear();
	}
	
private:
	std::string m_name;
	std::vector<double> m_times;
	bool m_justReported;
};