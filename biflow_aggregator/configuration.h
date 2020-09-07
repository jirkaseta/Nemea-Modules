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

    std::pair<aggregator::Field_config, int> parse_field(rapidxml::xml_node<> *option);
    aggregator::Sort_type get_sort_type(const char *input);
    aggregator::Field_type get_field_type(const char *input);
    int verify_field(aggregator::Field_config& field);
    std::vector<aggregator::Field_config> fields;

public:

    std::string out_tmplt;   

    void add_field_to_template(const std::string name);

    
    
    std::vector<aggregator::Field_config> get_cfg_fields() const noexcept;
    int set_timeout(const char *input);
    int set_flow_cache_size(const char *input);
    int parse_xml(const char *filename);

    Configuration();
    

    // TODO kotrola vice pouziti stejneho pole
};

#endif // CONFIGURATION_H