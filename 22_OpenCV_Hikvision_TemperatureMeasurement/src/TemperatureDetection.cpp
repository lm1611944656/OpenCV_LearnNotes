/*************************************************************************
 *
 *   Copyright (C), 2017-2027, BPG. Co., Ltd.
 *
 *   文件名称: TemperatureDetection.cpp
 *   软件模块: 应用模块
 *   版 本 号: 1.0
 *   生成日期: 2025-07-10
 *   作    者: lium
 *   功    能: 海康摄像头温度检测模块
 *
 *************************************************************************/

#include "TemperatureDetection.h"

CTemperatureDetection::CTemperatureDetection(const std::string &cameraInfo, const std::string &userName, const std::string &passwd)
    : cameraInfo_(cameraInfo), userName_(userName), passwd_(passwd),
    width_(0), height_(0), data_len_(0), scale_(0.0F), offset_(0.0F)
{
}

CTemperatureDetection::~CTemperatureDetection()
{
    if (curl)
    {
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

void CTemperatureDetection::setRequestHeaderOfUrl()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if (!curl)
    {
        std::cout << "初始化curl对象失败!" << std::endl;
        return;
    }

    curl_easy_setopt(curl, CURLOPT_URL, cameraInfo_.c_str());
    curl_easy_setopt(curl, CURLOPT_USERNAME, userName_.c_str()); // 用户名
    curl_easy_setopt(curl, CURLOPT_PASSWORD, passwd_.c_str());   // 密码
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);   // ✅ 启用 Digest 认证

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CTemperatureDetection::write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData_);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, CTemperatureDetection::header_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &responseData_);
}

void CTemperatureDetection::executeRequest(CURL *curl)
{
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        std::cerr << "请求失败: " << curl_easy_strerror(res) << std::endl;
        return;
    }
}

size_t CTemperatureDetection::write_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    TWriteData_t *data = static_cast<TWriteData_t *>(userdata);
    size_t total = size * nmemb;
    data->data.insert(data->data.end(), static_cast<char *>(ptr), static_cast<char *>(ptr) + total);
    return total;
}

size_t CTemperatureDetection::header_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    std::string header(static_cast<char *>(ptr), size * nmemb);
    // cout << "Header: " << header; // 打印所有接收到的头部信息

    std::string key = "Content-Type:";
    size_t pos = header.find(key);
    if (pos != std::string::npos)
    {
        std::string ct = header.substr(pos + key.length());
        ct.erase(remove_if(ct.begin(), ct.end(), [](char c)
                           { return c == '\r' || c == '\n'; }),
                 ct.end());
        TWriteData_t *data = static_cast<TWriteData_t *>(userdata);
        data->contentType = ct;
    }
    return size * nmemb;
}

std::string CTemperatureDetection::extract_boundary(const std::string &contentType)
{
    size_t pos = contentType.find("boundary=");
    if (pos == std::string::npos)
        return "";
    pos += 9;
    size_t end = contentType.find_first_of("; \"", pos);
    if (end == std::string::npos)
        end = contentType.length();
    std::string boundary = contentType.substr(pos, end - pos);
    if (boundary.front() == '"')
        boundary = boundary.substr(1, boundary.size() - 2); // 去掉引号
    return boundary;
}

std::vector<std::vector<char>> CTemperatureDetection::split_multipart(const std::vector<char> &data, const std::string &boundary)
{
    std::vector<std::vector<char>> parts;
    std::string delimiter = "--" + boundary;
    const char *p = data.data();
    size_t len = data.size();
    size_t pos = 0;

    while (pos < len)
    {
        size_t start = std::string(p + pos, len - pos).find(delimiter);
        if (start == std::string::npos)
            break;
        start += pos;
        pos = start + delimiter.length();

        // Find end of headers
        size_t headerEnd = std::string(p + pos, len - pos).find("\r\n\r\n");
        if (headerEnd == std::string::npos)
        {
            headerEnd = std::string(p + pos, len - pos).find("\n\n");
            if (headerEnd == std::string::npos)
                continue;
            headerEnd += pos + 2;
        }
        else
        {
            headerEnd += pos + 4;
        }

        // Find next boundary
        size_t end = std::string(p + headerEnd, len - headerEnd).find(delimiter);
        if (end == std::string::npos)
            end = len;
        else
            end += headerEnd;

        // Trim trailing CR/LF
        while (end > headerEnd && (p[end - 1] == '\n' || p[end - 1] == '\r'))
            end--;

        parts.push_back(std::vector<char>(p + headerEnd, p + end));
        pos = end;
    }

    // Remove the first dummy part if exists
    if (!parts.empty() && parts[0].empty())
        parts.erase(parts.begin());
    return parts;
}

float CTemperatureDetection::parse_float(const std::string &s)
{
    try
    {
        return stof(s);
    }
    catch (...)
    {
        return 0.0f;
    }
}

int CTemperatureDetection::parse_int(const std::string &s)
{
    try
    {
        return stoi(s);
    }
    catch (...)
    {
        return 0;
    }
}

void CTemperatureDetection::extract_json_values(const std::string &json, int &width, int &height, int &data_len, float &scale, float &offset)
{
    struct Field
    {
        const char *key;
        void *val;
        bool isFloat;
    };
    Field fields[] = {
        {"\"jpegPicWidth\":", &width, false},
        {"\"jpegPicHeight\":", &height, false},
        {"\"temperatureDataLength\":", &data_len, false},
        {"\"scale\":", &scale, true},
        {"\"offset\":", &offset, true}};

    for (auto &field : fields)
    {
        size_t pos = json.find(field.key);
        if (pos == std::string::npos)
            continue;
        pos += strlen(field.key);
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == ':'))
            pos++;
        size_t end = json.find_first_of(",}", pos);
        std::string val = json.substr(pos, end - pos);
        if (field.isFloat)
            *(float *)field.val = parse_float(val);
        else
            *(int *)field.val = parse_int(val);
    }
}

std::vector<float> CTemperatureDetection::parse_temperature(const std::vector<char> &data, int width, int height, int data_len, float scale, float offset)
{
    std::vector<float> tempsData;
    if (data_len != 2 && data_len != 4)
    {
        std::cout << "数据长度不是2字节, 或者4字节错误!" << std::endl;
        return {};
    };

    size_t expected_size = width * height * data_len;

    if (data.size() < expected_size)
    {
        std::cout << "data.size() < expected_size" << std::endl;
        return {};
    }

    for (int i = 0; i < width * height; ++i)
    {
        size_t idx = i * data_len;
        if (data_len == 2)
        {
            // 2字节：小端格式转换为 int16_t
            int16_t val = static_cast<int16_t>((static_cast<unsigned char>(data[idx + 1]) << 8) |
                                               static_cast<unsigned char>(data[idx]));
            float temp = val / scale + offset - 273.15f; // 转为摄氏度
            tempsData.push_back(temp);
        }
        else
        {
            // 4字节：IEEE 754 浮点数，需按大端处理
            uint32_t bytes = (static_cast<unsigned char>(data[idx + 3]) << 24) |
                             (static_cast<unsigned char>(data[idx + 2]) << 16) |
                             (static_cast<unsigned char>(data[idx + 1]) << 8) |
                             static_cast<unsigned char>(data[idx]);
            float temp;
            memcpy(&temp, &bytes, 4);
            tempsData.push_back(temp);
        }
    }
    return tempsData;
}

std::vector<float> CTemperatureDetection::getAreaAllTemperature()
{
    return this->tempsDataArr_;
}

TTemperaureData_t CTemperatureDetection::getAreaMaxTemperature()
{
    TTemperaureData_t areaMaxTemperature;

    // 找最大值元素指针
    auto max_it = std::max_element(tempsDataArr_.begin(), tempsDataArr_.end());

    // 获取一维索引
    int max_index = std::distance(tempsDataArr_.begin(), max_it);

    // 转换为二维坐标
    int max_x = max_index % this->width_; // 列
    int max_y = max_index / this->width_; // 行

    float max_temp = *max_it;

    /**结构体赋值 */
    areaMaxTemperature.temperaure = max_temp;
    areaMaxTemperature.x = max_x;
    areaMaxTemperature.y = max_y;
    return areaMaxTemperature;
}

TTemperaureData_t CTemperatureDetection::getAreaMinTemperature()
{
    TTemperaureData_t areaMinTemperature;

    auto min_it = std::min_element(tempsDataArr_.begin(), tempsDataArr_.end());
    int index = std::distance(tempsDataArr_.begin(), min_it);
    int x = index % this->width_;
    int y = index / this->width_;

    float min_temp = *min_it;

    /**结构体赋值 */
    areaMinTemperature.temperaure = min_temp;
    areaMinTemperature.x = x;
    areaMinTemperature.y = y;
    return areaMinTemperature;
}

float CTemperatureDetection::getAreaAverageTemperature()
{
    float average_temp = std::accumulate(tempsDataArr_.begin(), tempsDataArr_.end(), 0.0f) / tempsDataArr_.size();
    return average_temp;
}

TTemperatureStats_t CTemperatureDetection::calculateTemperatureStatsInRegion(int x_start, int y_start, int width, int height)
{
    TTemperatureStats_t stats = {0.0f, 0.0f, 0.0f};

    if (width <= 0 || height <= 0 || x_start < 0 || y_start < 0 ||
        x_start + width > this->width_ || y_start + height > this->height_) {
        std::cerr << "无效的区域参数" << std::endl;
        return stats; // 返回默认值
    }

    float sum = 0.0f;
    float maxTemp = -FLT_MAX;
    float minTemp = FLT_MAX;
    int count = 0;

    for (int y = y_start; y < y_start + height; ++y) {
        for (int x = x_start; x < x_start + width; ++x) {
            int index = y * this->width_ + x;
            if (index >= static_cast<int>(tempsDataArr_.size())) {
                std::cerr << "索引越界: index=" << index << ", size=" << tempsDataArr_.size() << std::endl;
                continue;
            }
            float temp = tempsDataArr_[index];
            sum += temp;
            if (temp > maxTemp) maxTemp = temp;
            if (temp < minTemp) minTemp = temp;
            ++count;
        }
    }

    if (count > 0) {
        stats.maxTemperaure = maxTemp;
        stats.minTemperaure = minTemp;
        stats.averageTemperaure = sum / count;
    } else {
        std::cerr << "没有有效温度数据" << std::endl;
    }

    return stats;
}

void CTemperatureDetection::sendRequestAndParseData()
{
    /**1,设置请求头 */
    setRequestHeaderOfUrl();

    /**2，发送请求 */
    executeRequest(this->curl);

    /**3，从响应头中，获取关键字“boundary” */
    std::string boundary = extract_boundary(responseData_.contentType);

    /**4，从响应体中，获取指定“boundary”关键字的内容 */
    std::vector<std::vector<char>> parts = split_multipart(responseData_.data, boundary);
    if (parts.size() < 3)
    {
        std::cerr << "非法的多部分响应" << std::endl;
        std::cout << "parts.size()" << parts.size() << std::endl;
    }
    if((parts.empty()) && (parts.size() < 2))
    {
        std::cout << "获取boundary" << std::endl;
        return;
    }
    std::cout << "boundary的个数: " << parts.size() << std::endl;

    /**5. 从第1个boundary中解析温度矩阵的信息 */
    std::string jsonData(parts[0].begin(), parts[0].end());
    extract_json_values(jsonData, this->width_, this->height_, this->data_len_, this->scale_, this->offset_);

    /**6. 从第3个boundary中解析温度矩阵的数据 */
    this->tempsDataArr_ =  parse_temperature(parts[2], this->width_, this->height_, this->data_len_, this->scale_, this->offset_);
    if (tempsDataArr_.empty())
    {
        std::cerr << "温度数据为空或解析失败！" << std::endl;
    }
}

/*************************************************************************
 * 改动历史纪录：
 * Revision 1.0, 2025-07-10, lium
 * describe: 初始创建.
 *************************************************************************/