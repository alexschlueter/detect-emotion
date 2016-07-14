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
    std::string getStringValue(const std::string &key, const std::string& defaultVal = "");
    int getIntValue(const std::string &key, int defaultVal = 0);
    float getFloatValue(const std::string &key, float defaultVal = 0);
    bool getBoolValue(const std::string &key, bool defaultVal = false);
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
/**
 * Loads the configuration from a file. Returns true on success, false otherweise.
 */
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
/**
 * Helper function to trim strings.
 */
std::string Configuration::trim(const std::string &str)
{
    size_t first = str.find_first_not_of(' ');
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last-first+1));
}
/**
 * Saves the configuration to a file. Returns true on success, false otherwise.
 */
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
/**
 * Stores value under key as a string.
 */
bool Configuration::store(const std::string &key, const std::string &value)
{
    std::string keyTrimmed = trim(key);
    std::string valueTrimmed = trim(value);
    if (keyTrimmed == "" || valueTrimmed == "")
        return false;

    storage[keyTrimmed] = valueTrimmed;
    return true;
}
/**
 * Stores value under key. Internally, the value is converted to a string.
 */
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
/**
 * Stores value under key. Internally the value is converted to a string.
 */
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
/**
 * Stores value under key. Internally the value is converted to a string.
 */
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

/**
 * Tries to find key in storage and return it's value as a string. If the key is not found,
 * the default  value is returned.
 */
std::string Configuration::getStringValue(const std::string &key, const std::string &defaultVal /* = "" */)
{
    auto iterator = storage.find(key);
    if(iterator == storage.end())
        return defaultVal;

    return iterator->second;
}
/**
 * Tries to find key in storage and return it's value as an integer. If the key is not found,
 * the default value is returned.
 */
int Configuration::getIntValue(const std::string &key, int defaultVal /* = 0 */)
{
    auto iterator = storage.find(key);
    if(iterator == storage.end())
        return defaultVal;

    try
    {
        return std::stoi(iterator->second);
    }
    catch(...)
    {
    }
    return defaultVal;
}
/**
 * Tries to find key in storage and return it's value as a float. If the key is not found,
 * the default value is returned.
 */
float Configuration::getFloatValue(const std::string &key, float defaultVal /* = 0 */)
{
    auto iterator = storage.find(key);
    if (iterator == storage.end())
        return defaultVal;

    try
    {
        return std::stof(iterator->second);
    }
    catch (...)
    {
    }
    return defaultVal;
}
/**
 * Tries to find key in storage and return it's value as a boolean. If the key is not found,
 * the default value is returned.
 */
bool Configuration::getBoolValue(const std::string &key, bool defaultVal /* = false */)
{
    auto iterator = storage.find(key);
    if (iterator == storage.end())
        return defaultVal;

    try
    {
        return (bool)std::stoi(iterator->second);
    }
    catch (...)
    {
    }
    return defaultVal;
}

int Configuration::size()
{
    return storage.size();
}
