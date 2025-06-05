#ifndef __OBJECTDETECTION_H__
#define __OBJECTDETECTION_H__


#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>


class YoloV5CPP{

public:
    explicit YoloV5CPP(const std::string &modeFile, const std::string &classNames);
    ~YoloV5CPP();

    void modelInferenceOfImg(const std::string &imgPath);

    void modelInferenceOfVideo(const std::string &videoPath);

private:
    const std::string model_path;
    const std::string labels_file;
};

#endif /**__OBJECTDETECTION_H__ */