#include "frame.hpp"

#include "compat/math.hpp"

#include <opencv2/imgproc.hpp>


Preview& Preview::operator=(const cv::Mat& frame)
{

    if (frame.channels() != 3)
    {
        eval_once(qDebug() << "tracker/pt: camera frame depth: 3 !=" << frame.channels());
        return *this;
    }

    const bool need_resize = frame.cols != frame_out.cols || frame.rows != frame_out.rows;
    if (need_resize)
        cv::resize(frame, frame_copy, cv::Size(frame_out.cols, frame_out.rows), 0, 0, cv::INTER_NEAREST);
    else
        frame.copyTo(frame_copy);

    return *this;
}

Preview::Preview(int w, int h)
{
    ensure_size(frame_out, w, h, CV_8UC4);
    ensure_size(frame_copy, w, h, CV_8UC3);

    frame_copy.setTo(cv::Scalar(0, 0, 0));
}

QImage Preview::get_bitmap()
{
    int stride = frame_out.step.p[0];

    if (stride < 64 || stride < frame_out.cols * 4)
    {
        eval_once(qDebug() << "bad stride" << stride
                           << "for bitmap size" << frame_copy.cols << frame_copy.rows);
        return QImage();
    }

    cv::cvtColor(frame_copy, frame_out, cv::COLOR_BGR2BGRA);

    return QImage((const unsigned char*) frame_out.data,
                  frame_out.cols, frame_out.rows,
                  stride,
                  QImage::Format_ARGB32);
}

void Preview::draw_head_center(pt_pixel_pos_mixin::f x, pt_pixel_pos_mixin::f y)
{
    auto [px_, py_] = pt_pixel_pos_mixin::to_pixel_pos(x, y, frame_copy.cols, frame_copy.rows);

    int px = iround(px_), py = iround(py_);

    constexpr int len = 9;

    static const cv::Scalar color(0, 255, 255);
    cv::line(frame_copy,
             cv::Point(px - len, py),
             cv::Point(px + len, py),
             color, 1);
    cv::line(frame_copy,
             cv::Point(px, py - len),
             cv::Point(px, py + len),
             color, 1);
}

void Preview::ensure_size(cv::Mat& frame, int w, int h, int type)
{
    if (frame.cols != w || frame.rows != h)
        frame = cv::Mat(h, w, type);
}
