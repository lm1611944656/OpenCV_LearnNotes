#include <iostream>
#include <string>
#include <vector>
#include <algorithm> // std::min, std::max
#include <sstream>   // std::stringstream
#include <cmath>     // stof, stoi
#include <cstring>   // memcpy
#include <cstdint>   // int16_t, uint32_t
#include <curl/curl.h>
#include <fstream>
#include <numeric> //（用于 std::accumulate）


// 用于存储响应数据及 Content-Type 类型
struct WriteData {
    std::vector<char> data;        // 响应主体数据
    std::string contentType;       // 头部中的 Content-Type 字段
};

// 写入响应体的回调函数
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    WriteData *data = static_cast<WriteData*>(userdata);
    size_t total = size * nmemb;
    data->data.insert(data->data.end(), static_cast<char*>(ptr), static_cast<char*>(ptr) + total);
    return total;
}

// 写入头部信息的回调函数，用于提取 Content-Type 和 boundary
size_t header_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    std::string header(static_cast<char*>(ptr), size * nmemb);
    //cout << "Header: " << header; // 打印所有接收到的头部信息

    std::string key = "Content-Type:";
    size_t pos = header.find(key);
    if (pos != std::string::npos) {
        std::string ct = header.substr(pos + key.length());
        ct.erase(remove_if(ct.begin(), ct.end(), [](char c) { return c == '\r' || c == '\n'; }), ct.end());
        WriteData *data = static_cast<WriteData*>(userdata);
        data->contentType = ct;
    }
    return size * nmemb;
}

// 从 Content-Type 提取 boundary 字符串
std::string extract_boundary(const std::string& contentType) {
    size_t pos = contentType.find("boundary=");
    if (pos == std::string::npos) return "";
    pos += 9;
    size_t end = contentType.find_first_of("; \"", pos);
    if (end == std::string::npos) end = contentType.length();
    std::string boundary = contentType.substr(pos, end - pos);
    if (boundary.front() == '"') boundary = boundary.substr(1, boundary.size()-2); // 去掉引号
    return boundary;
}

// 将 multipart 数据按照 boundary 分割为多个部分
std::vector<std::vector<char>> split_multipart(const std::vector<char>& data, const std::string& boundary) {
    std::vector<std::vector<char>> parts;
    std::string delimiter = "--" + boundary;
    const char *p = data.data();
    size_t len = data.size();
    size_t pos = 0;

    while (pos < len) {
        size_t start = std::string(p + pos, len - pos).find(delimiter);
        if (start == std::string::npos) break;
        start += pos;
        pos = start + delimiter.length();

        // Find end of headers
        size_t headerEnd = std::string(p + pos, len - pos).find("\r\n\r\n");
        if (headerEnd == std::string::npos) {
            headerEnd = std::string(p + pos, len - pos).find("\n\n");
            if (headerEnd == std::string::npos) continue;
            headerEnd += pos + 2;
        } else {
            headerEnd += pos + 4;
        }

        // Find next boundary
        size_t end = std::string(p + headerEnd, len - headerEnd).find(delimiter);
        if (end == std::string::npos) end = len;
        else end += headerEnd;

        // Trim trailing CR/LF
        while (end > headerEnd && (p[end-1] == '\n' || p[end-1] == '\r')) end--;

        parts.push_back(std::vector<char>(p + headerEnd, p + end));
        pos = end;
    }

    // Remove the first dummy part if exists
    if (!parts.empty() && parts[0].empty()) parts.erase(parts.begin());
    return parts;
}

// 安全地将字符串转为 float
float parse_float(const std::string& s) {
    try { return stof(s); } catch(...) { return 0.0f; }
}

// 安全地将字符串转为 int
int parse_int(const std::string& s) {
    try { return stoi(s); } catch(...) { return 0; }
}

// 从 JSON 字符串中提取指定字段的值
void extract_json_values(const std::string& json, int &width, int &height, int &data_len, float &scale, float &offset) {
    struct Field { const char *key; void *val; bool isFloat; };
    Field fields[] = {
        {"\"jpegPicWidth\":", &width, false},
        {"\"jpegPicHeight\":", &height, false},
        {"\"temperatureDataLength\":", &data_len, false},
        {"\"scale\":", &scale, true},
        {"\"offset\":", &offset, true}
    };

    for (auto &field : fields) {
        size_t pos = json.find(field.key);
        if (pos == std::string::npos) continue;
        pos += strlen(field.key);
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == ':')) pos++;
        size_t end = json.find_first_of(",}", pos);
        std::string val = json.substr(pos, end - pos);
        if (field.isFloat) *(float*)field.val = parse_float(val);
        else *(int*)field.val = parse_int(val);
    }
}

// 解析温度数据（支持 2 字节或 4 字节格式）
std::vector<float> parse_temperature(const std::vector<char>& data, int width, int height, int data_len, float scale, float offset) {
    std::vector<float> temps;
    if (data_len != 2 && data_len != 4) return temps;
    size_t expected_size = width * height * data_len;
    if (data.size() < expected_size) return temps;

    for (int i = 0; i < width * height; ++i) {
        size_t idx = i * data_len;
        if (data_len == 2) {
            // 2字节：小端格式转换为 int16_t
            int16_t val = static_cast<int16_t>((static_cast<unsigned char>(data[idx+1]) << 8) | 
                         static_cast<unsigned char>(data[idx]));
            float temp = val / scale + offset - 273.15f; // 转为摄氏度
            temps.push_back(temp);
        } else {
            // 4字节：IEEE 754 浮点数，需按大端处理
            uint32_t bytes = (static_cast<unsigned char>(data[idx+3]) << 24) |
                            (static_cast<unsigned char>(data[idx+2]) << 16) |
                            (static_cast<unsigned char>(data[idx+1]) << 8) |
                            static_cast<unsigned char>(data[idx]);
            float temp;
            memcpy(&temp, &bytes, 4);
            temps.push_back(temp);
        }
    }
    return temps;
}

// 获取指定区域的最高温、最低温和平均温
void get_temperature_stats(const std::vector<float>& temps, int width, int height,
                           int x, int y, int w, int h,
                           float& max_temp, float& min_temp, float& avg_temp) {
    max_temp = -INFINITY;
    min_temp = INFINITY;
    double sum_temp = 0.0;
    int count = 0;

    // 确保输入坐标有效
    int x_start = std::max(0, x);
    int y_start = std::max(0, y);
    int x_end = std::min(width, x + w);
    int y_end = std::min(height, y + h);

    for (int j = y_start; j < y_end; ++j) {
        for (int i = x_start; i < x_end; ++i) {
            float t = temps[j * width + i];
            max_temp = std::max(max_temp, t);
            min_temp = std::min(min_temp, t);
            sum_temp += t;
            ++count;
        }
    }

    if (count > 0)
        avg_temp = static_cast<float>(sum_temp / count);
    else
        avg_temp = 0.0f;
}

int main() {
    CURL *curl;
    CURLcode res;
    WriteData responseData;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(curl) {
        std::string url = "http://10.10.12.228/ISAPI/Thermal/channels/2/thermometry/jpegPicWithAppendData?format=json";

        // 设置请求选项
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_USERNAME, "admin");          // 用户名
        curl_easy_setopt(curl, CURLOPT_PASSWORD, "密码");      // 密码
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST); // ✅ 启用 Digest 认证

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback); 
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);      
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback); 
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &responseData);     

        // 执行请求
        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
            std::cerr << "请求失败: " << curl_easy_strerror(res) << std::endl;
        else {
            std::cout << "响应大小: " << responseData.data.size() << " 字节" << std::endl;

            /**从multipart数据中提取出，关键字 “boundary” */
            std::string boundary = extract_boundary(responseData.contentType);
            std::cout << "提取到 boundary: " << boundary << std::endl;

            /**在响应体中，将“boundary”关键字部分的内容提取出来 */
            std::vector<std::vector<char>> parts = split_multipart(responseData.data, boundary);
            if(parts.size() < 3){
                std::cerr << "非法的多部分响应" << std::endl;
                std::cout << "parts.size()" << parts.size() << std::endl;
            }
            std::cout << "boundary的个数: " << parts.size() << std::endl;

            // std::ofstream jpeg("./images/thermal_image.jpg", std::ios::binary);
            // jpeg.write(parts[1].data(), parts[1].size());
            // jpeg.close();
            // std::cout << "JPEG保存成功" << std::endl;


            if (!parts.empty() && parts.size() >= 2) {
                
                /**从第1个boundary中解析温度矩阵的信息 */
                std::string jsonData(parts[0].begin(), parts[0].end());
                int width = 0, height = 0, data_len = 0;
                float scale = 1.0f, offset = 0.0f;
                extract_json_values(jsonData, width, height, data_len, scale, offset);
                std::cout << "图像尺寸: " << width << "x" << height << std::endl;
                std::cout << "温度数据长度: " << data_len << std::endl;
                std::cout << "Scale: " << scale << ", Offset: " << offset << std::endl;

                /**查看第2个boundary */
                std::cout << "第二个 part 大小: " << parts[1].size() << " 字节" << std::endl;

                /**从第3个boundary中解析温度矩阵的数据 */
                std::vector<float> temps = parse_temperature(parts[2], width, height, data_len, scale, offset);
                if (!temps.empty()) {
                    float max_temp_area = *std::max_element(temps.begin(), temps.end());
                    float min_temp_area = *std::min_element(temps.begin(), temps.end());
                    float average_temp = std::accumulate(temps.begin(), temps.end(), 0.0f) / temps.size();

                    std::cout << "最高温度：" << max_temp_area << "℃" <<  std::endl;
                    std::cout << "最低温度：" << min_temp_area << "℃" << std::endl;
                    std::cout << "平均温度：" << average_temp << "℃" << std::endl;

                } else {
                    std::cerr << "温度数据为空或解析失败！" << std::endl;
                }
            } else {
                std::cerr << "无法解析 multipart 数据，分片数量不足。" << std::endl;
            }
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return 0;
}
