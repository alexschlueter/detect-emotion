#pragma once
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <map>

/**
 * Allows to load settings from a configuration file in the simple format key=value.
 */
class Configuration
{
public:
    Configuration();
    Configuration(const std::string &filepath);
    bool load(const std::string &filepath);
    bool save(const std::string &filepath);
    bool store(const std::string &key, const std::string &value);
    bool store(const std::string &key, const int &value);
    bool store(const std::string &key, const float &value);
    bool store(const std::string &key, const bool &value);
    std::string getStringValue(const std::string &key, const std::string& default = "");
    int getIntValue(const std::string &key, int default = 0);
    float getFloatValue(const std::string &key, float default = 0);
    bool getBoolValue(const std::string &key, bool default = false);
    int size();
private:
    std::string trim(const std::string &str);

    std::map<std::string, std::string> storage;
};

Configuration::Configuration()
{

}

Configuration::Configuration(const std::string &filepath) : Configuration()
{
    load(filepath);
}

bool Configuration::load(const std::string &filepath)
{
    std::ifstream file(filepath);
    if (!file)
    {
        return false;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    std::string line;
    while (std::getline(buffer, line))
    {
        std::string trimmedLine = trim(line);
        if (trimmedLine == "" || trimmedLine.find_first_of("#") == 0) // skip empty & comments
            continue;

        std::istringstream keyValuePair(trimmedLine);
        std::string key;
        if (std::getline(keyValuePair, key, '='))
        {
            std::string value;
            if (std::getline(keyValuePair, value))
                store(key, value);
        }
    }
    return true;
}

std::string Configuration::trim(const std::string &str)
{
    size_t first = str.find_first_not_of(' ');
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last-first+1));
}

bool Configuration::save(const std::string &filepath)
{
    std::ofstream out(filepath);
    for(auto keyValue : storage)
    {
        out << keyValue.first << "=" << keyValue.second << std::endl;
    }
    out.close();
    return true;
}

bool Configuration::store(const std::string &key, const std::string &value)
{
    std::string keyTrimmed = trim(key);
    std::string valueTrimmed = trim(value);
    if (keyTrimmed == "" || valueTrimmed == "")
        return false;

    storage[keyTrimmed] = valueTrimmed;
    return true;
}

bool Configuration::store(const std::string &key, const int &value)
{
    try
    {
        std::string valueString = std::to_string(value);
        return store(key, valueString);
    }
    catch(...)
    {
        return false;
    }
    return true;
}

bool Configuration::store(const std::string &key, const float &value)
{
    try
    {
        std::string valueString = std::to_string(value);
        return store(key, valueString);
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool Configuration::store(const std::string &key, const bool &value)
{
    try
    {
        std::string valueString = std::to_string((int)value);
        return store(key, valueString);
    }
    catch (...)
    {
        return false;
    }
    return true;
}

std::string Configuration::getStringValue(const std::string &key, const std::string &default /* = "" */)
{
    auto iterator = storage.find(key);
    if(iterator == storage.end())
        return default;

    return iterator->second;
}
int Configuration::getIntValue(const std::string &key, int default /* = 0 */)
{
    auto iterator = storage.find(key);
    if(iterator == storage.end())
        return default;

    try
    {
        return std::stoi(iterator->second);
    }
    catch(...)
    {
    }
    return default;
}
float Configuration::getFloatValue(const std::string &key, float default /* = 0 */)
{
    auto iterator = storage.find(key);
    if (iterator == storage.end())
        return default;

    try
    {
        return std::stof(iterator->second);
    }
    catch (...)
    {
    }
    return default;
}
bool Configuration::getBoolValue(const std::string &key, bool default /* = false */)
{
    auto iterator = storage.find(key);
    if (iterator == storage.end())
        return default;

    try
    {
        return (bool)std::stoi(iterator->second);
    }
    catch (...)
    {
    }
    return default;
}

int Configuration::size()
{
    return storage.size();
}