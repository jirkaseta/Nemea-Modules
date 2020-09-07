#ifndef KEY_TEMPLATE_H
#define KEY_TEMPLATE_H

#include <libtrap/trap.h>
#include <unirec/unirec.h>
#include <vector>

#include "xxhash.h"

class KeyTemplate {
    std::vector<std::pair<int, std::size_t>> key_fields_;
    std::size_t key_size_;
    

public:
    void update_template(int field_id, std::size_t field_size);
    void reset_template() noexcept;
    const std::vector<std::pair<int, std::size_t>> get_template_fields() noexcept;
    std::size_t get_template_size();
};

class FlowKey {
    uint8_t *data_;
    std::size_t size_;

public:
    void init(std::size_t key_size);
    void update(const void *src, std::size_t size);
    void reset();
    std::pair<void *, size_t> get_key() const;
    bool operator==(const FlowKey &other) const noexcept;

    FlowKey(const FlowKey &other);

    FlowKey()
    {   
    }
};

namespace std
{
    template<> struct hash<FlowKey>
    {
        std::size_t operator()(FlowKey const& key) const noexcept
        {
            auto flow_key = key.get_key(); 
            return static_cast<std::size_t>(XXH3_64bits(flow_key.first, flow_key.second));
        }
    };
}

#endif // KEY_TEMPLATE_H