#pragma once

#include <exception>
#include <string>

class LoadJsonException : public std::exception
{
  public:
    LoadJsonException(const std::string& message) : message(message)
    {
    }

    virtual const char* what() const noexcept override
    {
        return message.c_str();
    }

  private:
    std::string message;
};

class JsonConfigurationException : public std::exception
{
  public:
    JsonConfigurationException(const std::string& message) : message(message)
    {
    }

    virtual const char* what() const noexcept override
    {
        return message.c_str();
    }

  private:
    std::string message;
};
