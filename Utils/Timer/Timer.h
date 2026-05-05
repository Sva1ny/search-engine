#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>
#include <iostream>

class Timer
{
public:
    Timer()
    {
        m_start = std::chrono::high_resolution_clock::now();
    }
    ~Timer()
    {
    }
    void end()
    {
        m_end = std::chrono::high_resolution_clock::now();
        std::cout << "Time elapsed: " << std::chrono::duration_cast<std::chrono::microseconds>(m_end - m_start).count() << " microseconds" << std::endl;
    }

private:
    std::chrono::high_resolution_clock::time_point m_start;
    std::chrono::high_resolution_clock::time_point m_end;
};

#endif