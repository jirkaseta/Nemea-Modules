/**
 * @file agregator_fields.h
 * @author Pavel Siska (siska@cesnet.cz)
 * @brief 
 * @version 0.1
 * @date 31.8.2020
 *   
 * @copyright Copyright (c) 2020 CESNET
 */

#ifndef AGGREGATOR_H
#define AGGREGATOR_H

#include "flat_hash_map.h"
#include "key_template.h"

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
    SORTED_MERGE,
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
    template<typename T, typename K>
    int assign() noexcept;

    template<Field_type ag_type, typename T>
    int assign() noexcept;

    template<typename T>
    int assign() noexcept;

    template<typename T>
    int assign_append() noexcept;

protected:

    aggr_func ag_fnc;
    post_func post_proc_fnc;
    init_func init_fnc;
    std::size_t typename_size;

    int set_templates(const Field_type ag_type, const ur_field_type_t ur_f_type);
    int set_templates(const ur_field_type_t ur_f_type, const ur_field_type_t ur_sort_key_f_type);

public:
    std::size_t ag_data_size;
};

struct Flow_data {
    char *data;
    uint32_t cnt;
    time_t first;
    time_t last;
    bool reverse;

    void update(const time_t time_first, const time_t time_last) noexcept;
    void update(const time_t time_first, const time_t time_last, bool is_reverse) noexcept;
    void update(const uint32_t count) noexcept;

    Flow_data();
};


struct Field_config {
    std::string name;
    std::string reverse_name;
    Field_type type;
    std::string sort_name;
    Sort_type sort_type;
    char delimiter;
    std::size_t limit;
    bool to_output;
};

// Class to represent aggregation field.
class Field : public Field_config, public Field_template {
public:
    // ID of unirec field
    ur_field_id_t ur_field_id;
    ur_field_id_t ur_field_reverse_id;

    // only for SORTED_MERGE;
    ur_field_id_t ur_sort_key_id;
    ur_field_type_t ur_sort_key_type;

    Field(const Field_config cfg, const ur_field_id_t field_id, const ur_field_id_t rev_field_id);

    void init(void *tmplt_mem, const void *cfg);
    void aggregate(const void *src, void *dst);
    const void *post_processing(void *ag_data, std::size_t& typename_size, std::size_t& elem_cnt);
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

template<typename Key>
class Aggregator : public Fields {

public:
    ska::flat_hash_map<Key, Flow_data> flow_cache;
};

} // namespace aggregator

#endif // AGGREGATOR_H    