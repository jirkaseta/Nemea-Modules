/**
 * @file configuration.h
 * @author Pavel Siska (siska@cesnet.cz)
 * @brief 
 * @version 0.1
 * @date 31.8.2020
 * 
 * @copyright Copyright (c) 2020 CESNET
 */

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "aggregator.h"
#include "rapidxml.hpp"

#include <string>
#include <vector>

#include <unirec/unirec.h>

class Configuration {

    std::pair<aggregator::Field_config, bool> parse_field(rapidxml::xml_node<> *option);
    aggregator::Sort_type get_sort_type(const char *input);
    aggregator::Field_type get_field_type(const char *input);
    bool verify_field(aggregator::Field_config& field);
    std::vector<aggregator::Field_config> fields;

    int check_biflow();
    bool is_key_present(std::string key_name);

public:

    std::string out_tmplt;   

    void add_field_to_template(const std::string name);

    bool is_biflow_key;
    bool break_when_eof;
    
    std::vector<aggregator::Field_config> get_cfg_fields() const noexcept;
    int set_timeout(const char *input);
    int set_flow_cache_size(const char *input);
    int parse_xml(const char *filename, const char *identifier); 

    Configuration();

    // TODO kotrola vice pouziti stejneho pole
};

#endif // CONFIGURATION_H