/**
 * @file configuration.cpp
 * @author Pavel Siska (siska@cesnet.cz)
 * @brief 
 * @version 0.1
 * @date 31.8.2020
 * 
 * @copyright Copyright (c) 2020 CESNET
 */

#include "configuration.h"
#include "aggregator.h"
#include "aggregator_functions.h"
#include "key_template.h"

#include <iostream>
#include <vector>
#include <string.h>
#include <cstring>
#include <fstream>

using namespace rapidxml;

int Configuration::verify_field(aggregator:: Field_config& field)
{
    if (field.type == aggregator::INVALID_TYPE)
        return 1;
    if (field.sort_name.length() == 0 && field.type == aggregator::SORTED_APPEND)
        return 1;
    if (field.sort_type == aggregator::INVALID_SORT_TYPE && field.type == aggregator::SORTED_APPEND) 
        return 1;
    return 0;
}

void debug_print_cfg_field(aggregator::Field_config& field)
{
    std::cout << "----------------" << std::endl;
    std::cout << "Name: " << field.name << std::endl;
    std::cout << "Type: " << field.type << std::endl;
    std::cout << "Sort key: " << field.sort_name << std::endl;
    std::cout << "Sort type: " << field.sort_type << std::endl;
    std::cout << "Delimiter: " << field.delimiter << std::endl;
    std::cout << "Size: " << field.limit << std::endl;
}

std::pair<aggregator::Field_config, int> Configuration::parse_field(xml_node<> *xml_field)
{
    aggregator::Field_config field = {};

    for (xml_node<> *option = xml_field->first_node(); option; option = option->next_sibling()) {
        if (!strcmp(option->name(), "name")) {
            field.name = option->value();
        } else if (!strcmp(option->name(), "type")) {
            field.type = get_field_type(option->value());
        } else if (!strcmp(option->name(), "sort_key")) {
            field.sort_name = option->value();
        } else if (!strcmp(option->name(), "sort_type")) {
            field.sort_type = get_sort_type(option->value());
        } else if (!strcmp(option->name(), "delimiter")) {
            if (std::strlen(option->value()) != 1) {
                std::cerr << "Invalid delimiter length. Given: " << std::strlen(option->value()) << ", expected: 1." << std::endl;
                return std::make_pair(field, 1);
            }
            field.delimiter = option->value()[0];
        } else if (!strcmp(option->name(), "size")) {
            field.limit = std::stoi(option->value(), NULL);
            if (field.limit == 0) {
                std::cerr << "Invalid size format. Given: " << option->value() << ", expected: unsigned number." << std::endl;
                return std::make_pair(field, 1);;
            }
        } else {
            std::cerr << "Invlaid file format. Expected 'name|type|[sort_key|sort_type|delimiter|size]', given '" << option->name() << "'" << std::endl;
            return std::make_pair(field, 1);;
        }
    }

    debug_print_cfg_field(field);
    return std::make_pair(field, verify_field(field));
}

int Configuration::parse_xml(const char *filename)
{
    xml_document<> doc;
    std::ifstream file(filename, std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) {
        std::cerr << "Unable to read file " << filename << std::endl;
        return 1;
    }

    doc.parse<0>(buffer.data());
    if (strcmp(doc.first_node()->name(), "aggregator")) {
        std::cerr << "Invlaid file format. Expected 'aggregator', given '" << doc.first_node()->name() << "'" << std::endl;
        return 1;
    }

    for (xml_node<> *xml_field = doc.first_node()->first_node(); xml_field; xml_field = xml_field->next_sibling()) {
        std::cout << xml_field->name() <<  "\n";
        if (strcmp(xml_field->name(), "field")) {
            std::cerr << "Invlaid file format. Expected 'field', given '" << xml_field->name() << "'" << std::endl;
            return 1;
        }
        std::pair<aggregator::Field_config, int> p_field = parse_field(xml_field);
        if (p_field.second == 1)
            return 1;
        
        fields.emplace_back(p_field.first);
    }

    return 0;
}

Configuration::Configuration()
{
    out_tmplt = "TIME_FIRST,TIME_LAST,COUNT";
}

void Configuration::add_field_to_template(const std::string name)
{
    out_tmplt.append(",");
    out_tmplt.append(name);
}

std::vector<aggregator::Field_config> Configuration::get_cfg_fields() const noexcept
{
    return fields;
}

aggregator::Sort_type Configuration::get_sort_type(const char *input)
{
    if (!strcmp(input, "ASCENDING")) return aggregator::ASCENDING;
    if (!strcmp(input, "DESCENDING")) return aggregator::DESCENDING;
    std::cerr << "Invalid sort type field. Given: " << input << ", Expected: ASCENDING|DESCENDING." << std::endl;
    return aggregator::INVALID_SORT_TYPE;
}

aggregator::Field_type Configuration::get_field_type(const char *input)
{
    std::cout << "FIELD TYPE\n";
    if (!strcmp(input, "KEY")) return aggregator::KEY;
    if (!strcmp(input, "SUM")) return aggregator::SUM;
    if (!strcmp(input, "MIN")) return aggregator::MIN;
    if (!strcmp(input, "MAX")) return aggregator::MAX;
    if (!strcmp(input, "AVG")) return aggregator::AVG;
    if (!strcmp(input, "BITAND")) return aggregator::BIT_AND;
    if (!strcmp(input, "APPEND")) return aggregator::APPEND;
    if (!strcmp(input, "SORTED_APPEND")) return aggregator::SORTED_APPEND;
    std::cerr << "Invalid type field. Given: " << input << ", Expected: KEY|SUM|MIN|MAX|AVG|BITAND|APPEND|SORTED_APPEND." << std::endl;
    return aggregator::INVALID_TYPE;
}

int Configuration::set_timeout(const char *input)
{
    // TODO
    return 0;
}

int Configuration::set_flow_cache_size(const char *input)
{
    // TODO
    return 0;
}