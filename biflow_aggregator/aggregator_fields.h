/**
 * @file agregator_fields.h
 * @author Pavel Siska (siska@cesnet.cz)
 * @brief 
 * @version 0.1
 * @date 31.8.2020
 *   
 * @copyright Copyright (c) 2020 CESNET
 */

#ifndef AGGREGATOR_TEMPLATES_H
#define AGGREGATOR_TEMPLATES_H

#include <cassert>
#include <unirec/unirec.h>
#include <string>
#include <vector>

namespace aggregator {

// Type of field defining aggregation function.
enum Field_type {
    KEY,
    SUM,
    AVG,
    MIN,
    MAX,
    BIT_AND,
    APPEND,
    SORTED_APPEND,
    INVALID_TYPE,
};

// Type of sort order
enum Sort_type {
    ASCENDING,
    DESCENDING,
    INVALID_SORT_TYPE
};

using uint128_t = unsigned __int128;
using aggr_func = void (*)(const void *, void *);
using post_func = const void *(*)(void *, std::size_t&);
using init_func = void (*)(void *, const void *);

class Field_template {
    aggr_func ag_fnc;
    post_func post_proc_fnc;
    init_func init_fnc;
    std::size_t typename_size;
    std::size_t ag_data_size;

public:

    void init(void *tmplt_mem, const void *cfg);
    void aggregate(const void *src, void *dst);
    const void *post_processing(void *ag_data, std::size_t& typename_size, std::size_t& elem_cnt);
};

struct Flow_data {
    char *data;
    uint32_t cnt;
    time_t first;
    time_t last;

    void update(const time_t time_first, const time_t time_last) noexcept;
    void update(const uint32_t count) noexcept;

    Flow_data();
};


struct Field_config {
    std::string name;
    Field_type type;
    std::string sort_name;
    Sort_type sort_type;
    char delimiter;
    std::size_t limit;
};

// Class to represent aggregation field.
class Field : public Field_config, private Field_template {
public:
    // ID of unirec field
    ur_field_id_t ur_field_id;

    // only for SORTED_APPEND;
    ur_field_id_t ur_sort_key_id;
    ur_field_type_t ur_sort_key_type;

    std::size_t data_size;

    Field(const Field_config cfg, const ur_field_id_t field_id);
};

// Class to represent all aggregation fields and memory that fields need.
class Fields {
    std::size_t offset_;
    std::vector<std::pair<Field, std::size_t>> fields_;

    std::vector<void *> allocated_pointers_; // TODO

public:

    Fields()
    {
    }

    void add_field(Field field);
    
    void reset_fields() noexcept;
    void *allocate_memory();  
    std::vector<std::pair<Field, std::size_t>> get_fields() noexcept;
};

} // namespace aggregator

#endif // AGGREGATOR_TEMPLATES_H    