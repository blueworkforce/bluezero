#ifndef B0__UTILS__THREAD_NAME_H__INCLUDED
#define B0__UTILS__THREAD_NAME_H__INCLUDED

#include <string>
//#include <thread>

void set_thread_name(const char *threadName);

#if 0
void set_thread_name(std::thread *thread, const char *threadName);
#endif

std::string get_thread_name();

#endif // B0__UTILS__THREAD_NAME_H__INCLUDED
