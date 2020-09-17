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

bool Configuration::verify_field(aggregator:: Field_config& field)
{
    if (field.type == aggregator::INVALID_TYPE)
        return false;
    if (field.sort_name.length() == 0 && field.type == aggregator::SORTED_MERGE)
        return false;
    if (field.sort_type == aggregator::INVALID_SORT_TYPE && field.type == aggregator::SORTED_MERGE) 
        return false;

    // check duplications
    for (auto cfg_field: fields) {
        if (field.name.compare(cfg_field.name) == 0) {
            std::cerr << "Duplicit field name (" << field.name <<")" << std::endl;
            return false;
        }
    }

    return true;
}

void debug_print_cfg_field(aggregator::Field_config& field)
{
    std::cout << "----------------" << std::endl;
    std::cout << "Name: " << field.name << std::endl;
    std::cout << "Reverse name: " << field.reverse_name << std::endl;
    std::cout << "Type: " << field.type << std::endl;
    std::cout << "Sort key: " << field.sort_name << std::endl;
    std::cout << "Sort type: " << field.sort_type << std::endl;
    std::cout << "Delimiter: " << field.delimiter << std::endl;
    std::cout << "Size: " << field.limit << std::endl;
    std::cout << "----------------" << std::endl;
}

std::pair<aggregator::Field_config, bool> Configuration::parse_field(xml_node<> *xml_field)
{
    aggregator::Field_config field = {};

    for (xml_node<> *option = xml_field->first_node(); option; option = option->next_sibling()) {
        if (!strcmp(option->name(), "name")) {
            field.name = option->value();
        } else if (!strcmp(option->name(), "reverse_name")) {
            field.reverse_name = option->value();
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
            std::cerr << "Invlaid file format. Expected 'name|type|[reverse_name|sort_key|sort_type|delimiter|size]', given '" << option->name() << "'" << std::endl;
            return std::make_pair(field, 1);;
        }
    }
    field.to_output = true;
    //debug_print_cfg_field(field);
    return std::make_pair(field, verify_field(field));
}

bool Configuration::is_key_present(std::string key_name)
{
    for (auto cfg_field : fields) {
        if (!cfg_field.name.compare(key_name))
            return true; 
    }
    return false;
}

int Configuration::check_biflow()
{
    const std::vector<std::string> biflow_keys = {"SRC_IP", "DST_IP", "SRC_PORT", "DST_PORT", "PROTOCOL"};
    int ret = 0;
    
    for (auto key : biflow_keys) {
        if (is_key_present(key) == false) {
            return 0; // not a biflow key
        }
    }

    for (auto field : fields) {
        if (!field.name.compare("SRC_IP")) {
            if (field.reverse_name.compare("DST_IP")) {
                std::cerr << "Invalid combination of name/reverse name. Expected SRC_IP/DST_IP" << std::endl;
                ret = 1;
            }
        } else if (!field.name.compare("DST_IP")) {
            if (field.reverse_name.compare("SRC_IP")) {
                std::cerr << "Invalid combination of name/reverse name. Expected DST_IP/SRC_IP" << std::endl;
                ret = 1;
            }
        } else if (!field.name.compare("SRC_PORT")) {
            if (field.reverse_name.compare("DST_PORT")) {
                std::cerr << "Invalid combination of name/reverse name. Expected SRC_PORT/DST_PORT" << std::endl;
                ret = 1;
            }
        } else if (!field.name.compare("DST_PORT")) {
            if (field.reverse_name.compare("SRC_PORT")) {
                std::cerr << "Invalid combination of name/reverse name. Expected DST_PORT/SRC_PORT" << std::endl;
                ret = 1;
            }
        }
    }

    for (auto field : fields) {
        if (!field.reverse_name.empty() && is_key_present(field.reverse_name) == false) { // create a reverse field
            aggregator::Field_config f_cfg = {};
            f_cfg.name = field.reverse_name;
            f_cfg.reverse_name = field.name;
            f_cfg.type = field.type;
            f_cfg.sort_name = field.sort_name;
            f_cfg.sort_type = field.sort_type;
            f_cfg.delimiter = field.delimiter;
            f_cfg.limit = field.limit;
            f_cfg.to_output = false;
            fields.emplace_back(f_cfg);
        }
    }

    is_biflow_key = true;
    return ret;
}

int Configuration::parse_xml(const char *filename, const char *identifier) 
{
    xml_document<> doc;
    std::ifstream file(filename, std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    bool found = false;


    std::vector<char> buffer(size + 1);
    if (!file.read(buffer.data(), size)) {
        std::cerr << "Unable to read file " << filename << std::endl;
        return 1;
    }

    doc.parse<0>(buffer.data());
    if (strcmp(doc.first_node()->name(), "aggregator")) {
        std::cerr << "Invlaid file format. Expected 'aggregator', given '" << doc.first_node()->name() << "'" << std::endl;
        return 1;
    }

    for (xml_node<> *id = doc.first_node()->first_node(); id; id = id->next_sibling()) {
        if (strcmp(id->name(), "id")) {
            std::cerr << "Invlaid file format. Expected 'id', given '" << id->name() << "'" << std::endl;
            return 1;
        }

        rapidxml::xml_attribute<>* attr = id->first_attribute("name");
        if (attr == nullptr) {
            std::cerr << "Invalid file format. Expected '<id name=\"NAME\">'" << std::endl;
            return 1;
        } else {
            if (strcmp(attr->value(), identifier))
                continue;
        }

        found = true;

        for (xml_node<> *xml_field = id->first_node(); xml_field; xml_field = xml_field->next_sibling()) {
            if (strcmp(xml_field->name(), "field")) {
                std::cerr << "Invlaid file format. Expected 'field', given '" << xml_field->name() << "'" << std::endl;
                return 1;
            }
            std::pair<aggregator::Field_config, bool> p_field = parse_field(xml_field);
            if (p_field.second == false)
                return 1;
            
            fields.emplace_back(p_field.first);
        }
        break;
    }

    if (!found) {
        std::cerr << "Invalid file format. No ID (" << identifier << ") found." << std::endl;
        return 1;
    }

    return check_biflow();
}

Configuration::Configuration()
{
    out_tmplt = "TIME_FIRST,TIME_LAST,COUNT";
    break_when_eof = false;
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
    if (!strcmp(input, "KEY")) return aggregator::KEY;
    if (!strcmp(input, "SUM")) return aggregator::SUM;
    if (!strcmp(input, "MIN")) return aggregator::MIN;
    if (!strcmp(input, "MAX")) return aggregator::MAX;
    if (!strcmp(input, "AVG")) return aggregator::AVG;
    if (!strcmp(input, "BITAND")) return aggregator::BIT_AND;
    if (!strcmp(input, "APPEND")) return aggregator::APPEND;
    if (!strcmp(input, "SORTED_MERGE")) return aggregator::SORTED_MERGE;
    std::cerr << "Invalid type field. Given: " << input << ", Expected: KEY|SUM|MIN|MAX|AVG|BITAND|APPEND|SORTED_MERGE." << std::endl;
    return aggregator::INVALID_TYPE;
}

int Configuration::set_flow_cache_size(const char *input)
{
    // TODO
    return 0;
}

int Configuration::set_active_timeout(const char *input)
{
    t_active = stoul(input);
    return 0;
}

int Configuration::set_passive_timeout(const char *input)
{
    t_passive = stoul(input);
    return 0;
}

void Configuration::set_eof_break()
{
    break_when_eof = true;
}