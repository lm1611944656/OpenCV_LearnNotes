/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: main.cpp
*   软件模块: 图像处理
*   版 本 号: 1.0
*   生成日期: 2025-05-29
*   作    者: lium
*   功    能: 图像处理主函数
* 
************************************************************************/

#include "imgclassification.h"
#include <thread>
#include "threadpool.h"
#include "objectdetection.h"

/**未使用线程池 */
#if 0
int main(int argc, char **argv){

    const std::string modelFile = "weigths/yolov5s_DoorStatus_20250620.onnx";
    std::string classFile = "weigths/yolov5s_DoorStatus_20250620.txt";
    std::string srcImgPath = "data/test_power/DoorStatus_583.jpg";

    // 
    auto clsObj = std::make_unique<CImgClassification>(modelFile, classFile);

    TClassificationResult clsResult;
    clsResult = clsObj->imgClsTask(srcImgPath);
    std::cout << "检测到的类别：" << clsResult.className << std::endl;
    std::cout << "检测到的类别概率：" << clsResult.probability << std::endl;

    for(int i = 0; i < clsResult.allProbability.size(); i++){
        std::cout << "类别" << i << ":"<< clsResult.allProbability[i] << std::endl;
    }

    return 0;
}
#endif


/**未使用线程池 */
#if 1
int main(int argc, char **argv){

    const std::string modelFile = "weigths/yolov5s_Hat_C3_CBAM_20250709.onnx";
    std::string classFile = "weigths/classes_Hat.txt";
    std::string srcImgPath = "data/Hat5.jpg";

    /**创建一个检测对象 */
    auto objDetect = std::make_unique<CObjDetection>(modelFile, classFile);

    std::vector<TDetectResult> detResult;
    detResult = objDetect->imgDetectTask(srcImgPath);

    for (const auto& detection : detResult) {
        // 打印类别名称
        std::cout << "Detected: " << detection.className << std::endl;

        // 打印概率
        std::cout << ", Probability: " << detection.probability;

        // 打印边界框的位置和大小
        std::cout << ", Box: (" 
                << detection.box.x << ", "  // 边界框左上角x坐标
                << detection.box.y << ", "  // 边界框左上角y坐标
                << detection.box.width << ", "  // 边界框宽度
                << detection.box.height << ")"  // 边界框高度
                << std::endl;
    }

    // cv::namedWindow("Result", cv::WINDOW_AUTOSIZE);
    cv::waitKey(0);
    cv::destroyAllWindows();
    return 0;
}
#endif


/**使用线程池做目标检测任务 */
#if 0
int main(int argc, char **argv){

    const std::string modelFile = "weigths/yolov5s_DoorStatus_20250620.onnx";
    std::string classFile = "weigths/yolov5s_DoorStatus_20250620.txt";
    std::string srcImgPath4 = "data/test_power/DoorStatus_1075.jpg";
    std::string srcImgPath = "data/test_power/DoorStatus_3481.jpg";
    std::string srcImgPath2 = "data/test_power/DoorStatus_3481.jpg";

    /**创建线程池对象 */
    ThreadPool pool(6);

    /**创建检测对象 */
    auto objDetect = std::make_unique<CObjDetection>(modelFile, classFile);

    /**第一个检测任务 */
    auto futureResult = pool.enqueue(&CObjDetection::imgDetectTask, objDetect.get(), srcImgPath2);
    try {
        std::vector<TDetectResult> result = futureResult.get();  // 会阻塞直到线程执行完成
        
        for (const auto& detection : result) {
            // 打印类别名称
            std::cout << "Detected: " << detection.className;

            // 打印概率
            std::cout << ", Probability: " << detection.probability;

            // 打印边界框的位置和大小
            std::cout << ", Box: (" 
                    << detection.box.x << ", "  // 边界框左上角x坐标
                    << detection.box.y << ", "  // 边界框左上角y坐标
                    << detection.box.width << ", "  // 边界框宽度
                    << detection.box.height << ")"  // 边界框高度
                    << std::endl;
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "线程任务异常: " << ex.what() << std::endl;
    }

    std::cout << "*******************************************" << std::endl;

    /**第二个检测任务 */
    auto futureResult2 = pool.enqueue(&CObjDetection::imgDetectTask, objDetect.get(), srcImgPath2);
    try {
        std::vector<TDetectResult> result2 = futureResult2.get();  // 会阻塞直到线程执行完成
        for (const auto& detection : result2) {
            // 打印类别名称
            std::cout << "Detected: " << detection.className;

            // 打印概率
            std::cout << ", Probability: " << detection.probability;

            // 打印边界框的位置和大小
            std::cout << ", Box: (" 
                    << detection.box.x << ", "  // 边界框左上角x坐标
                    << detection.box.y << ", "  // 边界框左上角y坐标
                    << detection.box.width << ", "  // 边界框宽度
                    << detection.box.height << ")"  // 边界框高度
                    << std::endl;
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "线程任务异常: " << ex.what() << std::endl;
    }

    // cv::waitKey(0);
    // cv::destroyAllWindows();

    return 0;
}
#endif

/**使用线程池做图像分类任务 */
#if 0
int main(int argc, char **argv){

    const std::string modelFile = "./weigths/yolov5_classs.onnx";
    std::string classFile = "./weigths/yolov5_classs.txt";
    std::string srcImgPath = "./data/DoorClosed_35.jpg";
    std::string srcImgPath2 = "./data/DoorClosed_35.jpg";
    std::string srcImgPath3 = "./data/DoorClosed_35.jpg";
    std::string srcImgPath4 = "./data/DoorClosed_35.jpg";
    std::string srcImgPath5 = "./data/DoorClosed_35.jpg";
    std::string srcImgPath6 = "./data/DoorClosed_35.jpg";

    auto clsObj = std::make_unique<CImgClassification>(modelFile, classFile);

    ThreadPool pool(40);

    while(true) {
        // 线程1
        auto futureResult = pool.enqueue(&CImgClassification::imgClsTask, clsObj.get(), srcImgPath);
        try {
            TClassificationResult result = futureResult.get();  // 会阻塞直到线程执行完成

            std::cout << "预测类别: " << result.className << std::endl;
            std::cout << "置信度: " << result.probability << std::endl;

            std::cout << "所有类别的概率：" << std::endl;
            for (size_t i = 0; i < result.allProbability.size(); ++i) {
                std::cout << "Class " << i << ": " << result.allProbability[i] << std::endl;
            }
        }
        catch (const std::exception& ex) {
            std::cerr << "线程任务异常: " << ex.what() << std::endl;
        }

        // 线程2
        auto futureResult2 = pool.enqueue(&CImgClassification::imgClsTask, clsObj.get(), srcImgPath2);
        try {
            TClassificationResult result = futureResult2.get();  // 会阻塞直到线程执行完成

            std::cout << "预测类别: " << result.className << std::endl;
            std::cout << "置信度: " << result.probability << std::endl;

            std::cout << "所有类别的概率：" << std::endl;
            for (size_t i = 0; i < result.allProbability.size(); ++i) {
                std::cout << "Class " << i << ": " << result.allProbability[i] << std::endl;
            }
        }
        catch (const std::exception& ex) {
            std::cerr << "线程任务异常: " << ex.what() << std::endl;
        }

        // 线程3
        auto futureResult3 = pool.enqueue(&CImgClassification::imgClsTask, clsObj.get(), srcImgPath3);
        try {
            TClassificationResult result = futureResult3.get();  // 会阻塞直到线程执行完成

            std::cout << "预测类别: " << result.className << std::endl;
            std::cout << "置信度: " << result.probability << std::endl;

            std::cout << "所有类别的概率：" << std::endl;
            for (size_t i = 0; i < result.allProbability.size(); ++i) {
                std::cout << "Class " << i << ": " << result.allProbability[i] << std::endl;
            }
        }
        catch (const std::exception& ex) {
            std::cerr << "线程任务异常: " << ex.what() << std::endl;
        }

        // 线程4
        auto futureResult4 = pool.enqueue(&CImgClassification::imgClsTask, clsObj.get(), srcImgPath4);
        try {
            TClassificationResult result = futureResult4.get();  // 会阻塞直到线程执行完成

            std::cout << "预测类别: " << result.className << std::endl;
            std::cout << "置信度: " << result.probability << std::endl;

            std::cout << "所有类别的概率：" << std::endl;
            for (size_t i = 0; i < result.allProbability.size(); ++i) {
                std::cout << "Class " << i << ": " << result.allProbability[i] << std::endl;
            }
        }
        catch (const std::exception& ex) {
            std::cerr << "线程任务异常: " << ex.what() << std::endl;
        }

        // 线程5
        auto futureResult5 = pool.enqueue(&CImgClassification::imgClsTask, clsObj.get(), srcImgPath5);
        try {
            TClassificationResult result = futureResult5.get();  // 会阻塞直到线程执行完成

            std::cout << "预测类别: " << result.className << std::endl;
            std::cout << "置信度: " << result.probability << std::endl;

            std::cout << "所有类别的概率：" << std::endl;
            for (size_t i = 0; i < result.allProbability.size(); ++i) {
                std::cout << "Class " << i << ": " << result.allProbability[i] << std::endl;
            }
        }
        catch (const std::exception& ex) {
            std::cerr << "线程任务异常: " << ex.what() << std::endl;
        }

        // 线程6
        auto futureResult6 = pool.enqueue(&CImgClassification::imgClsTask, clsObj.get(), srcImgPath6);
        try {
            TClassificationResult result = futureResult6.get();  // 会阻塞直到线程执行完成

            std::cout << "预测类别: " << result.className << std::endl;
            std::cout << "置信度: " << result.probability << std::endl;

            std::cout << "所有类别的概率：" << std::endl;
            for (size_t i = 0; i < result.allProbability.size(); ++i) {
                std::cout << "Class " << i << ": " << result.allProbability[i] << std::endl;
            }
        }
        catch (const std::exception& ex) {
            std::cerr << "线程任务异常: " << ex.what() << std::endl;
        }

        // 线程7
        auto futureResult7 = pool.enqueue(&CImgClassification::imgClsTask, clsObj.get(), srcImgPath6);
        try {
            TClassificationResult result = futureResult7.get();  // 会阻塞直到线程执行完成

            std::cout << "预测类别: " << result.className << std::endl;
            std::cout << "置信度: " << result.probability << std::endl;

            std::cout << "所有类别的概率：" << std::endl;
            for (size_t i = 0; i < result.allProbability.size(); ++i) {
                std::cout << "Class " << i << ": " << result.allProbability[i] << std::endl;
            }
        }
        catch (const std::exception& ex) {
            std::cerr << "线程任务异常: " << ex.what() << std::endl;
        }

        // 线程8
        auto futureResult8 = pool.enqueue(&CImgClassification::imgClsTask, clsObj.get(), srcImgPath6);
        try {
            TClassificationResult result = futureResult8.get();  // 会阻塞直到线程执行完成

            std::cout << "预测类别: " << result.className << std::endl;
            std::cout << "置信度: " << result.probability << std::endl;

            std::cout << "所有类别的概率：" << std::endl;
            for (size_t i = 0; i < result.allProbability.size(); ++i) {
                std::cout << "Class " << i << ": " << result.allProbability[i] << std::endl;
            }
        }
        catch (const std::exception& ex) {
            std::cerr << "线程任务异常: " << ex.what() << std::endl;
        }

        // 线程9
        auto futureResult9 = pool.enqueue(&CImgClassification::imgClsTask, clsObj.get(), srcImgPath6);
        try {
            TClassificationResult result = futureResult9.get();  // 会阻塞直到线程执行完成

            std::cout << "预测类别: " << result.className << std::endl;
            std::cout << "置信度: " << result.probability << std::endl;

            std::cout << "所有类别的概率：" << std::endl;
            for (size_t i = 0; i < result.allProbability.size(); ++i) {
                std::cout << "Class " << i << ": " << result.allProbability[i] << std::endl;
            }
        }
        catch (const std::exception& ex) {
            std::cerr << "线程任务异常: " << ex.what() << std::endl;
        }

        // 线程10
        auto futureResult10 = pool.enqueue(&CImgClassification::imgClsTask, clsObj.get(), srcImgPath6);
        try {
            TClassificationResult result = futureResult10.get();  // 会阻塞直到线程执行完成

            std::cout << "预测类别: " << result.className << std::endl;
            std::cout << "置信度: " << result.probability << std::endl;

            std::cout << "所有类别的概率：" << std::endl;
            for (size_t i = 0; i < result.allProbability.size(); ++i) {
                std::cout << "Class " << i << ": " << result.allProbability[i] << std::endl;
            }
        }
        catch (const std::exception& ex) {
            std::cerr << "线程任务异常: " << ex.what() << std::endl;
        }

        // 线程11
        auto futureResult11 = pool.enqueue(&CImgClassification::imgClsTask, clsObj.get(), srcImgPath6);
        try {
            TClassificationResult result = futureResult11.get();  // 会阻塞直到线程执行完成

            std::cout << "预测类别: " << result.className << std::endl;
            std::cout << "置信度: " << result.probability << std::endl;

            std::cout << "所有类别的概率：" << std::endl;
            for (size_t i = 0; i < result.allProbability.size(); ++i) {
                std::cout << "Class " << i << ": " << result.allProbability[i] << std::endl;
            }
        }
        catch (const std::exception& ex) {
            std::cerr << "线程任务异常: " << ex.what() << std::endl;
        }

        // 线程12
        auto futureResult12 = pool.enqueue(&CImgClassification::imgClsTask, clsObj.get(), srcImgPath6);
        try {
            TClassificationResult result = futureResult12.get();  // 会阻塞直到线程执行完成

            std::cout << "预测类别: " << result.className << std::endl;
            std::cout << "置信度: " << result.probability << std::endl;

            std::cout << "所有类别的概率：" << std::endl;
            for (size_t i = 0; i < result.allProbability.size(); ++i) {
                std::cout << "Class " << i << ": " << result.allProbability[i] << std::endl;
            }
        }
        catch (const std::exception& ex) {
            std::cerr << "线程任务异常: " << ex.what() << std::endl;
        }

        // 线程13
        auto futureResult13 = pool.enqueue(&CImgClassification::imgClsTask, clsObj.get(), srcImgPath6);
        try {
            TClassificationResult result = futureResult13.get();  // 会阻塞直到线程执行完成

            std::cout << "预测类别: " << result.className << std::endl;
            std::cout << "置信度: " << result.probability << std::endl;

            std::cout << "所有类别的概率：" << std::endl;
            for (size_t i = 0; i < result.allProbability.size(); ++i) {
                std::cout << "Class " << i << ": " << result.allProbability[i] << std::endl;
            }
        }
        catch (const std::exception& ex) {
            std::cerr << "线程任务异常: " << ex.what() << std::endl;
        }

        // 线程14
        auto futureResult14 = pool.enqueue(&CImgClassification::imgClsTask, clsObj.get(), srcImgPath6);
        try {
            TClassificationResult result = futureResult14.get();  // 会阻塞直到线程执行完成

            std::cout << "预测类别: " << result.className << std::endl;
            std::cout << "置信度: " << result.probability << std::endl;

            std::cout << "所有类别的概率：" << std::endl;
            for (size_t i = 0; i < result.allProbability.size(); ++i) {
                std::cout << "Class " << i << ": " << result.allProbability[i] << std::endl;
            }
        }
        catch (const std::exception& ex) {
            std::cerr << "线程任务异常: " << ex.what() << std::endl;
        }

        // 线程15
        auto futureResult15 = pool.enqueue(&CImgClassification::imgClsTask, clsObj.get(), srcImgPath6);
        try {
            TClassificationResult result = futureResult15.get();  // 会阻塞直到线程执行完成

            std::cout << "预测类别: " << result.className << std::endl;
            std::cout << "置信度: " << result.probability << std::endl;

            std::cout << "所有类别的概率：" << std::endl;
            for (size_t i = 0; i < result.allProbability.size(); ++i) {
                std::cout << "Class " << i << ": " << result.allProbability[i] << std::endl;
            }
        }
        catch (const std::exception& ex) {
            std::cerr << "线程任务异常: " << ex.what() << std::endl;
        }

        // 线程16
        auto futureResult16 = pool.enqueue(&CImgClassification::imgClsTask, clsObj.get(), srcImgPath6);
        try {
            TClassificationResult result = futureResult16.get();  // 会阻塞直到线程执行完成

            std::cout << "预测类别: " << result.className << std::endl;
            std::cout << "置信度: " << result.probability << std::endl;

            std::cout << "所有类别的概率：" << std::endl;
            for (size_t i = 0; i < result.allProbability.size(); ++i) {
                std::cout << "Class " << i << ": " << result.allProbability[i] << std::endl;
            }
        }
        catch (const std::exception& ex) {
            std::cerr << "线程任务异常: " << ex.what() << std::endl;
        }

        // 线程17
        auto futureResult17 = pool.enqueue(&CImgClassification::imgClsTask, clsObj.get(), srcImgPath6);
        try {
            TClassificationResult result = futureResult17.get();  // 会阻塞直到线程执行完成

            std::cout << "预测类别: " << result.className << std::endl;
            std::cout << "置信度: " << result.probability << std::endl;

            std::cout << "所有类别的概率：" << std::endl;
            for (size_t i = 0; i < result.allProbability.size(); ++i) {
                std::cout << "Class " << i << ": " << result.allProbability[i] << std::endl;
            }
        }
        catch (const std::exception& ex) {
            std::cerr << "线程任务异常: " << ex.what() << std::endl;
        }
    }
    return 0;
}

#endif



/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-05-29, lium
* describe: 初始创建.
*************************************************************************/


