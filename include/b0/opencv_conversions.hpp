#include <opencv2/core/core.hpp>

namespace camera_msgs {

static bool
ImageToMat(const Image &img, cv::Mat &mat)
{
    cv::Mat tmp(img.height(), img.width(), img.type(), (void*)img.data().data());
    tmp.copyTo(mat);
    return true;
}

static bool
ImageFromMat(Image &img, const cv::Mat &mat)
{
    std::vector<char> data;
    data.resize(mat.total() * mat.elemSize());
    if(mat.isContinuous())
    {
        memcpy(&data[0], mat.data, data.size());
    }
    else
    {
        const size_t rowsize = mat.cols * mat.elemSize();
        for(int i = 0; i < mat.rows; i++)
            memcpy(&data[i * rowsize], mat.ptr(i), rowsize);
    }
    img.set_width(mat.cols);
    img.set_height(mat.rows);
    img.set_type(mat.type());
    img.set_data((void*)&data[0], data.size());
    return true;
}

} // namespace camera_msgs

static bool
s_recv(zmq::socket_t &socket, cv::Mat &mat)
{
    camera_msgs::Image img;
    if(!s_recv(socket, img)) return false;
    return camera_msgs::ImageToMat(img, mat);
}

static bool
s_send(zmq::socket_t &socket, const cv::Mat &mat)
{
    camera_msgs::Image img;
    camera_msgs::ImageFromMat(img, mat);
    return s_send(socket, img);
}

