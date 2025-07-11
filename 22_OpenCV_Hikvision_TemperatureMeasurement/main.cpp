#include <memory>
#include "TemperatureDetection.h"


int main(int argc, char **argv)
{
    const std::string cameraInfo = "http://10.10.12.228/ISAPI/Thermal/channels/2/thermometry/jpegPicWithAppendData?format=json";
    const std::string userName = "admin";
    const std::string passwd = "密码";
    std::unique_ptr<CTemperatureDetection> tempDetect = std::make_unique<CTemperatureDetection>(cameraInfo, userName, passwd);
    
    /**发送请求，并且解析响应 */
    tempDetect->sendRequestAndParseData();

    /**获取区域平均温度值 */
    float averageTemp = tempDetect->getAreaAverageTemperature();
    std::cout << "区域平均温度值为：" << averageTemp << std::endl;

    /**获取区域的最高温度值 */
    TTemperaureData_t areaMaxTemperature;
    areaMaxTemperature = tempDetect->getAreaMaxTemperature();
    std::cout << "区域最高温度值为：" << areaMaxTemperature.temperaure 
    << " (x, y)-->" << "(" << areaMaxTemperature.x << ", " << areaMaxTemperature.y << ")" << std::endl;
    
    /**获取区域的最低温度值 */
    TTemperaureData_t areaMinTemperature;
    areaMinTemperature = tempDetect->getAreaMinTemperature();
    std::cout << "区域最低温度值为：" << areaMinTemperature.temperaure 
    << " (x, y)-->" << "(" << areaMinTemperature.x << ", " << areaMinTemperature.y << ")" << std::endl;
    
    /**获取整个区域的温度值 */
    std::vector<float> areaAllTemperature;
    areaAllTemperature = tempDetect->getAreaAllTemperature();
    std::cout << "温度数据点数：" << areaAllTemperature.size() << std::endl;
    
    // 获取某个区域（比如从 (120, 5)，大小 50x50）的温度统计信息
    TTemperatureStats_t stats = tempDetect->calculateTemperatureStatsInRegion(120, 5, 50, 50);
    std::cout << "Max Temp: " << stats.maxTemperaure << std::endl;
    std::cout << "Min Temp: " << stats.minTemperaure << std::endl;
    std::cout << "Avg Temp: " << stats.averageTemperaure << std::endl;

    return 0;
}