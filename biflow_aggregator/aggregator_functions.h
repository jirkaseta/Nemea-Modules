/**
 * @file agregator_functions.h
 * @author Pavel Siska (siska@cesnet.cz)
 * @brief 
 * @version 0.1
 * @date 31.8.2020
 *   
 * @copyright Copyright (c) 2020 CESNET
 */

#ifndef AGGREGATOR_FUNCTIONS_H
#define AGGREGATOR_FUNCTIONS_H

#include "aggregator.h"

#include <iostream>
#include <limits>
#include <unirec/unirec.h>

namespace aggregator {

/*
 * Structure of unirec array input.
 */
struct ur_array_data {
    std::size_t cnt_elements;
    const void *ptr_first;
    const void *sort_key;
};

/*
 * Basic template data structure that store variable of given type T.
 */ 
template <typename T>
struct Basic_data {
    T data; 

    static inline void init(void *mem, const void *cfg) noexcept
    {
        Basic_data<T> *basic = new(mem) Basic_data<T>();
        if (cfg != nullptr)
            basic->data = std::numeric_limits<T>::max();
    }
};

/*
 * Structure used to store data for average function.
 */
template <typename T>
struct Average_data : Basic_data<T> {
    uint32_t cnt;

    static inline void init(void *mem, const void *cfg) noexcept
    {
        (void) cfg;
        new(mem) Average_data<T>();
    }

    static inline const void *postprocessing(void *mem, std::size_t& elem_cnt) noexcept
    {
        Average_data<T> *avg = static_cast<Average_data<T>*>(mem);
        avg->data /= avg->cnt;
        return static_cast<void *>(&avg->data);
    }
};

struct Config_append {
    std::size_t limit;
    char delimiter;
};

/*
 * Structure used to store data for append function.
 */
template<typename T>
struct Append_data : Config_append {
    std::vector<T> data;
    
    static inline void init(void *mem, const void *cfg)
    {
        Append_data<T> *append = new(mem) Append_data<T>();
        const Config_append *config = static_cast<const Config_append *>(cfg);
        append->limit = config->limit;
        append->delimiter = config->limit;
        append->data.reserve(config->limit);
    }

    static inline const void *postprocessing(void *mem, std::size_t& elem_cnt) noexcept
    {
        Append_data<T> *append = static_cast<Append_data<T>*>(mem);
        elem_cnt = append->data.size(); 
        return append->data.data();
    }
};

struct Config_sorted_append {
    std::size_t limit;
    char delimiter;
    Sort_type sort_type;
};

/*
 * Structure used to store data for sorted append function.
 */
template <typename T, typename K>
struct Sorted_append_data : Config_sorted_append {
    std::vector<std::pair<std::vector<T>,K>> data;
    std::vector<T> result;
    
    static inline void init(void *mem, const void *cfg)
    {
        Sorted_append_data<T, K> *sorted_append = new(mem) Sorted_append_data<T, K>();
        const Config_sorted_append *config = static_cast<const Config_sorted_append *>(cfg);
        sorted_append->limit = config->limit;
        sorted_append->sort_type = config->sort_type;
        sorted_append->delimiter = config->delimiter;
        sorted_append->result.reserve(config->limit);
    }


    static inline const void *postprocessing(void *mem, std::size_t& elem_cnt)
    {
        Sorted_append_data<T, K> *sorted_append = static_cast<Sorted_append_data<T, K>*>(mem);
        Sort_type sort_type = sorted_append->sort_type;
        sort(sorted_append->data.begin(), sorted_append->data.end(), [&sort_type](const std::pair<std::vector<T>,K>& a, const std::pair<std::vector<T>,K>& b) -> bool { 
            if (sort_type == ASCENDING)
                return a.second > b.second;
            else
                return a.second < b.second;
            }); 

        for (auto it = sorted_append->data.begin(); it != sorted_append->data.end(); it++) {
            if (sorted_append->result.size() == sorted_append->limit)
                break;
            if (sorted_append->result.size() + it->first.size() <= sorted_append->limit)
                sorted_append->result.insert(sorted_append->result.end(), it->first.begin(), it->first.end());
            else
                sorted_append->result.insert(sorted_append->result.end(), it->first.begin(), it->first.begin() + (sorted_append->limit - sorted_append->result.size()));
        }

        elem_cnt = sorted_append->result.size();
        return sorted_append->result.data();
    }    
};

/**
 * Makes sum of values stored on src and dst pointers from given type T.
 * @tparam T template type variable.
 * @param [in] src pointer to source of new data.
 * @param [in,out] dst pointer to already stored data which will be updated (modified).
 */
template<typename T>
inline void sum(const void *src, void *dst) noexcept
{
    Basic_data<T> *sum = static_cast<Basic_data<T>*>(dst);
    sum->data += *(static_cast<const T*>(src));
}

/**
 * Store min value from values stored on src and dst pointers from given type T.
 * @tparam T template type variable.
 * @param [in] src pointer to source of new data.
 * @param [in,out] dst pointer to already stored data which will be updated (modified).
 */
template<typename T>
inline void min(const void *src, void *dst) noexcept
{
    Basic_data<T> *min = static_cast<Basic_data<T>*>(dst);
    if (*(static_cast<const T*>(src)) < min->data)
        min->data = *(static_cast<const T*>(src));
}

/**
 * Store max value from values stored on src and dst pointers from given type T.
 * @tparam T template type variable.
 * @param [in] src pointer to source of new data.
 * @param [in,out] dst pointer to already stored data which will be updated (modified).
 */
template<typename T>
inline void max(const void *src, void *dst) noexcept
{
    Basic_data<T> *max = static_cast<Basic_data<T>*>(dst);
    if (*(static_cast<const T*>(src)) > max->data)
        max->data = *(static_cast<const T*>(src));
}

/**
 * Store bitwise AND value from values stored on src and dst pointers from given type T.
 * @tparam T template type variable.
 * @param [in] src pointer to source of new data.
 * @param [in,out] dst pointer to already stored data which will be updated (modified).
 */
template <typename T>
inline void bitwise_and(const void *src, void *dst) noexcept
{
    Basic_data<T> *bit_and = static_cast<Basic_data<T>*>(dst);
    bit_and->data &= *(static_cast<const T*>(src));
}

/**
 * Makes sum of values stored on src and dst pointers from given type T.
 * @tparam T template type variable.
 * @param [in] src pointer to source of new data.
 * @param [in,out] dst pointer to already stored data which will be updated (modified).
 */
template<typename T>
inline void avg(const void *src, void *dst) noexcept
{
    Average_data<T> *avg = static_cast<Average_data<T>*>(dst);
    sum<T>(src, &avg->data);
    avg->cnt++;
}

/**
 * Append values stored on src pointers to dst pointer from given type T.
 * @tparam T template type variable.
 * @param [in] src pointer to source of new data.
 * @param [in,out] dst pointer to already stored data which will be updated (modified).
 */
template <typename T>
inline void append(const void *src, void *dst) noexcept
{
    Append_data<T> *append = static_cast<Append_data<T>*>(dst);
    const ur_array_data *src_data = (static_cast<const ur_array_data*>(src));
    std::size_t appended_data_size = append->data.size();
    
    if (appended_data_size == append->limit)
        return;

    if (std::is_same<T, char>::value) {
        if (appended_data_size + src_data->cnt_elements + 1 > append->limit)
            return;
        append->data.insert(append->data.end(), static_cast<const T*>(src_data->ptr_first), \
            static_cast<const T*>(src_data->ptr_first) + src_data->cnt_elements);
        T *delimiter = (T *)&append->delimiter;
        append->data.emplace_back(*delimiter);
        return;
    }
    
    if (appended_data_size + src_data->cnt_elements > append->limit)
        append->data.insert(append->data.end(), static_cast<const T*>(src_data->ptr_first), \
            static_cast<const T*>(src_data->ptr_first) + append->limit - appended_data_size);
    else
        append->data.insert(append->data.end(), static_cast<const T*>(src_data->ptr_first), \
            static_cast<const T*>(src_data->ptr_first) + src_data->cnt_elements);
}

template <typename T, typename K>
inline void sorted_append(const void *src, void *dst) noexcept
{
    Sorted_append_data<T, K> *sorted_append = static_cast<Sorted_append_data<T, K>*>(dst);
    const ur_array_data *src_data = (static_cast<const ur_array_data*>(src));

    std::vector<T> t_data;
    t_data.insert(t_data.begin(), static_cast<const T*>(src_data->ptr_first), \
        static_cast<const T*>(src_data->ptr_first) + src_data->cnt_elements);

    sorted_append->data.emplace_back(std::make_pair(t_data, *((K*)src_data->sort_key)));
}

} // namespace aggregator

#endif // AGGREGATOR_FUNCTIONS_H
