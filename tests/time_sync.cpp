#include <b0/utils/time_sync.h>

#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

#include <iostream>
#include <string>

// unit-test for the time sync algorithm
// (does not cover time synchronization between nodes)

namespace b0
{

// adds an offset to hardware clock

class TimeSync_TEST : public TimeSync
{
public:
    TimeSync_TEST(int64_t offset, double speed, double max_slope) : TimeSync(max_slope), t0_(TimeSync::hardwareTimeUSec()), offset_(offset), speed_(speed) {}
    int64_t hardwareTimeUSec() const {return offset_ + speed_ * (TimeSync::hardwareTimeUSec() - t0_) + t0_;}
protected:
    int64_t t0_;
    int64_t offset_;
    double speed_;
};

}

// exit codes
#define E_SUCCESS 0
#define E_INVALID_ARGS 1
#define E_INVALID_TEST 2
#define E_TEST_FAILED 3

inline int64_t sec(double s) { return s * 1000 * 1000; }

int run_test(const char *name, bool (*f)(int, char**), int argc, char **argv)
{
    std::cout << "running test '" << name << "'..." << std::endl;
    bool result = false;
    try
    {
        result = f(argc, argv);
        std::cout << "test '" << name << "' " << (result ? "PASSED" : "FAILED") << std::endl;
    }
    catch(std::exception &ex)
    {
        std::cout << "test '" << name << "' EXCEPTION: " << ex.what() << std::endl;
    }
    return result ? E_SUCCESS : E_TEST_FAILED;
}

bool test_offset_recover(int argc, char **argv)
{
    double max_slope = 0.5;
    int64_t offset = sec(5);
    double speed = 1.0;
    b0::TimeSync c(max_slope);
    b0::TimeSync_TEST s(offset, speed, max_slope);

    int64_t test_end_time = offset * 1.2 / max_slope + c.hardwareTimeUSec(); // run for the required time to adjust clock + 20%
    int64_t error = 0;

    while(c.hardwareTimeUSec() < test_end_time)
    {
        int64_t server_time = s.hardwareTimeUSec();
        c.updateTime(server_time);
        error = c.timeUSec() - server_time;
        std::cout << "server_time=" << server_time << ", error=" << error << std::endl;
        boost::this_thread::sleep_for(boost::chrono::seconds{1});
    }

    return std::abs(error) < sec(0.05);
}

bool test_clock_tracking(int argc, char **argv)
{
    if(argc != 4)
    {
        std::cerr << "usage: " << argv[0] << " <max-slope> <test-clock speed> <expected bool result>" << std::endl;
        return false;
    }

    double max_slope = boost::lexical_cast<double>(argv[1]);
    int64_t offset = sec(0);
    double speed = boost::lexical_cast<double>(argv[2]);
    bool expected = boost::lexical_cast<bool>(argv[3]);
    b0::TimeSync c(max_slope);
    b0::TimeSync_TEST s(offset, speed, max_slope);

    int64_t test_end_time = c.hardwareTimeUSec() + sec(5);
    int64_t error = 0;

    while(c.hardwareTimeUSec() < test_end_time)
    {
        int64_t server_time = s.hardwareTimeUSec();
        c.updateTime(server_time);
        error = c.timeUSec() - server_time;
        std::cout << "server_time=" << server_time << ", error=" << error << std::endl;
        boost::this_thread::sleep_for(boost::chrono::seconds{1});
    }

    // if it will be able to track the clock, the final offset will be constant at approximately (1 - speed) seconds
    int64_t error_bound = sec((1 - speed) * 1.05 /* 5% tolerance */);
    return !(expected ^ (std::abs(error) < std::abs(error_bound)));
}

#define BEGIN_TEST_LIST if(0) {}
#define ADD_TEST(n) else if(test_name_arg == #n) {return run_test(#n, &test_##n, argc-1, argv+1);}
#define END_TEST_LIST else {std::cerr << "invalid test name: '" << test_name_arg << std::endl; return E_INVALID_TEST;}

int main(int argc, char **argv)
{
    b0::init(argc, argv);

    if(argc < 2)
    {
        std::cerr << "usage: " << argv[0] << " <test_name>" << std::endl;
        return E_INVALID_ARGS;
    }

    std::string test_name_arg(argc > 1 ? argv[1] : "");

    BEGIN_TEST_LIST
    ADD_TEST(offset_recover)
    ADD_TEST(clock_tracking)
    END_TEST_LIST
}

