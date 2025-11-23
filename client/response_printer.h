// response_printer.h
#pragma once
#include "deserializer.h"
#include <iostream>

class ResponsePrinter
{
public:
    static void print(const Response &response, int indent = 0)
    {
        std::string indent_str(indent, ' ');

        switch (response.type)
        {
        case ResponseType::SIMPLE_STRING:
            std::cout << indent_str << "Simple String: " << response.simple_string << std::endl;
            break;

        case ResponseType::ERROR:
            std::cout << indent_str << "Error: " << response.simple_string << std::endl;
            break;

        case ResponseType::INTEGER:
            std::cout << indent_str << "Integer: " << response.integer << std::endl;
            break;

        case ResponseType::BULK_STRING:
            if (response.is_null)
            {
                std::cout << indent_str << "Bulk String: NULL" << std::endl;
            }
            else
            {
                std::cout << indent_str << "Bulk String: \"" << response.bulk_string << "\"" << std::endl;
            }
            break;

        case ResponseType::ARRAY:
            if (response.is_null)
            {
                std::cout << indent_str << "Array: NULL" << std::endl;
            }
            else
            {
                std::cout << indent_str << "Array [" << response.array.size() << " elements]:" << std::endl;
                for (const auto &element : response.array)
                {
                    print(element, indent + 2);
                }
            }
            break;

        case ResponseType::NULL_BULK_STRING:
            std::cout << indent_str << "Null Bulk String" << std::endl;
            break;
        }
    }

    static void print_raw_and_parsed(const std::string &raw_data)
    {
        std::cout << "Raw data: ";
        for (char c : raw_data)
        {
            if (c == '\r')
                std::cout << "\\r";
            else if (c == '\n')
                std::cout << "\\n";
            else
                std::cout << c;
        }
        std::cout << std::endl;

        try
        {
            Response response = Deserializer::parse(raw_data);
            std::cout << "Parsed response:" << std::endl;
            print(response, 2);
        }
        catch (const std::exception &e)
        {
            std::cout << "Parse error: " << e.what() << std::endl;
        }
        std::cout << "------------------------" << std::endl;
    }
};