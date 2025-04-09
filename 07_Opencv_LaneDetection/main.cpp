#include <iostream>
#include <opencv2/opencv.hpp>

/**参考地址：https://blog.csdn.net/Zero___Chen/article/details/119666454 */

/**读取图像 */
cv::Mat readImg(const std::string &imgPath){
    if(imgPath.empty()){
        throw std::invalid_argument("图像路径为空!");
    }

    cv::Mat srcImg = cv::imread(imgPath, cv::IMREAD_COLOR);
    if(srcImg.empty()){
        throw std::runtime_error("读取图像失败!");
    }
    return srcImg;
}   

/** 提取感兴趣的区域 */
void getROI(cv::Mat &srcImg, cv::Mat &ROIImg){
    if(srcImg.empty()){
        throw std::invalid_argument("传递参数为NULL!");
    }

    /**获取原始图像的尺寸 */
    int width = srcImg.cols;
    int height = srcImg.rows;

    /** 指定感兴趣区域的坐标点 */
    std::vector<cv::Point> pts;
    cv::Point ptA((width / 8) * 2, (height / 20) * 19);
	cv::Point ptB((width / 8) * 2, (height / 8) * 7);
	cv::Point ptC((width / 10) * 4, (height / 5) * 3);
	cv::Point ptD((width / 10) * 5, (height / 5) * 3);
	cv::Point ptE((width / 8) * 7, (height / 8) * 7);
	cv::Point ptF((width / 8) * 7, (height / 20) * 19);
    pts.push_back(ptA);
    pts.push_back(ptB);
    pts.push_back(ptC);
    pts.push_back(ptD);
    pts.push_back(ptE);
    pts.push_back(ptF);

    /**创建一张黑色图像 */
    cv::Mat blockImg = cv::Mat::zeros(srcImg.size(), srcImg.type());

    /**在黑色图像上，创建一个多边形(多边形内的颜色为纯白色) */
    cv::fillPoly(blockImg, pts, cv::Scalar::all(255));
    
    /**将拥有多边行的形状的黑色图像复制(盖)到ROIImg上 */
    srcImg.copyTo(ROIImg, blockImg);
}

/** 将感兴趣的区域转换为二值图像 */
cv::Mat convertToBinaryImage(cv::Mat &srcImg){
    if(srcImg.empty()){
        throw std::invalid_argument("传递参数为NULL!");
    }

    /** RGB转灰度图 */
    cv::Mat grayImg;
    cv::cvtColor(srcImg, grayImg, cv::COLOR_BGR2GRAY);

    /**将灰度图转换为二值图像(高于180的全部转换为最大值， 最大值为255) */
    cv::Mat binaryImage;
    cv::threshold(grayImg, binaryImage, 180, 255, cv::THRESH_BINARY);
    return binaryImage;
}

/**检测车道线 */
void detectRoadLine(cv::Mat &srcImg, cv::Mat &binaryImage){
    if (srcImg.empty() || binaryImage.empty()) {
        throw std::invalid_argument("传递参数为空！");
    }

    /**在二值图像中检测不为零的点 */
    std::vector<cv::Point> left_point;
    std::vector<cv::Point> right_point;
    /**左侧车道线 */
	for (int i = 0; i < binaryImage.cols / 2; i++)
	{
		for (int j = 0; j < binaryImage.rows; j++)
		{
			if (binaryImage.at<uchar>(j, i) == 255)
			{
				left_point.push_back(cv::Point(i, j));

			}
		}
	}

    /**右侧车道线 */
	for (int i = binaryImage.cols / 2; i < binaryImage.cols; i++)
	{
		for (int j = 0; j < binaryImage.rows; j++)
		{
			if (binaryImage.at<uchar>(j, i) == 255)
			{
				right_point.push_back(cv::Point(i, j));
			}
		}
	}

    /**车道线绘制 */
    if(left_point.size() && right_point.size()){
        /**指定顶部点和底部点 */
        cv::Point B_L = (left_point[0]);
		cv::Point T_L = (left_point[left_point.size() - 1]);
		cv::Point T_R = (right_point[0]);
		cv::Point B_R = (right_point[right_point.size() - 1]);

		circle(srcImg, B_L, 10, cv::Scalar(0, 0, 255), -1);
		circle(srcImg, T_L, 10, cv::Scalar(0, 255, 0), -1);
		circle(srcImg, T_R, 10, cv::Scalar(255, 0, 0), -1);
		circle(srcImg, B_R, 10, cv::Scalar(0, 255, 255), -1);

		line(srcImg, cv::Point(B_L), cv::Point(T_L), cv::Scalar(0, 255, 0), 10);
		line(srcImg, cv::Point(T_R), cv::Point(B_R), cv::Scalar(0, 255, 0), 10);

		std::vector<cv::Point> pts;
		pts = { B_L ,T_L ,T_R ,B_R };
		fillPoly(srcImg, pts, cv::Scalar(133, 230, 238));
    }

}

int main(int argc, const char **argv){
    std::string srcImgPath = "data/lane.jpeg";
    /**读取图像*/
    cv::Mat srcImg = readImg(srcImgPath);

    /** 获取感兴趣的区域 */
    cv::Mat ROIImg;
    getROI(srcImg, ROIImg);

    /** 转换为二值图像 */
    cv::Mat binaryImage = convertToBinaryImage(ROIImg);

    /**绘制车道线 */
    detectRoadLine(srcImg, binaryImage);

    cv::namedWindow("detectLoadLine", cv::WINDOW_AUTOSIZE);
    cv::imshow("detectLoadLine", srcImg);

    cv::waitKey(0);
    cv::destroyAllWindows();
    return 0;
}