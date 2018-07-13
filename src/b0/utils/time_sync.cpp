#include <b0/utils/time_sync.h>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace b0
{

TimeSync::TimeSync(double max_slope)
{
    target_offset_ = 0;
    last_offset_time_ = hardwareTimeUSec();
    last_offset_value_ = 0;
    max_slope_ = 0.5;
}

TimeSync::~TimeSync()
{
}

int64_t TimeSync::hardwareTimeUSec() const
{
    static boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
    boost::posix_time::ptime t = boost::posix_time::microsec_clock::local_time();
    return (t - epoch).total_microseconds();
}

int64_t TimeSync::timeUSec()
{
    return hardwareTimeUSec() + constantRateAdjustedOffset();
}

int64_t TimeSync::constantRateAdjustedOffset()
{
    boost::mutex::scoped_lock lock(mutex_);

    int64_t offset_delta = target_offset_ - last_offset_value_;
    int64_t slope_time = abs(offset_delta) / max_slope_;
    int64_t t = hardwareTimeUSec() - last_offset_time_;
    if(t >= slope_time)
        return target_offset_;
    else
        return last_offset_value_ + offset_delta * t / slope_time;
}

void TimeSync::updateTime(int64_t remoteTime)
{
    int64_t last_offset_value = constantRateAdjustedOffset();
    int64_t local_time = hardwareTimeUSec();

    {
        boost::mutex::scoped_lock lock(mutex_);

        last_offset_value_ = last_offset_value;
        last_offset_time_ = local_time;
        target_offset_ = remoteTime - local_time;
    }
}

} // namespace b0

