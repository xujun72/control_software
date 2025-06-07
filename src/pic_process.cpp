#include "config.hpp"
#include "pic_process.hpp"


//修改图片名称
void pic_rename(std::string newname,std::string oldname)
{
    std::rename(oldname.c_str(), newname.c_str());
}

/*
算法返回有故障信息的图片,图片格式为巡检发送的图片格式
文件名：线号_列车编号_相机号_当前站_下一站_终点站_当前公里标（m）
+距离公里标的位置（m）_备用_备用_时间戳(从1970到现在的毫秒数)_YYYYMMDDHHmmSS###_XXXX（图片序号）.jpg
*/

uint32_t pic_distance(std::string& pic_name)
{
    uint32_t dist;
    std::vector<std::string> result;
    std::stringstream ss(pic_name);
    std::string item;

    while (std::getline(ss, item, '_')) {
        result.push_back(item);
    }

    //pic_name 格式不正确，返回0
    if(result.size() != 13)
    {
        return 0;
    }
    dist = static_cast<uint32_t>(std::stoul(result[6]))+static_cast<uint32_t>(std::stoul(result[7]));

    return dist;
}

//替换file名称中距离数据
std::string pic_filename_updatedistance(std::string& pic_name,uint32_t value)
{
    std::vector<std::string> parts;
    std::stringstream ss(pic_name);
    std::string segment;

    // 使用'_'拆分字符串
    while (std::getline(ss, segment, '_')) {
        parts.push_back(segment);
    }

    // 至少需要12段以上，否则格式不对
    if (parts.size() < 12) {
        return pic_name; // 或者抛出异常
    }

    // 替换第7段（下标6）为新的 value 值
    parts[6] = std::to_string(value);

    // 重新组合为字符串
    std::string result = parts[0];
    for (size_t i = 1; i < parts.size(); ++i) {
        result += "_" + parts[i];
    }

    return result;
}

// std::string str;         // 声明但不赋初值
// char* cstr = (char*)"hello world"; 
// str = cstr;                      // 从 char* 指针赋值
// str.assign(cstr);               // 等效于上面的赋值
// str.assign(cstr, strlen(cstr)); // 指定长度赋值（更安全）