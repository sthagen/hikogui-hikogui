// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "formula_node.hpp"

namespace tt {

struct formula_map_literal_node final : formula_node {
    formula_vector keys;
    formula_vector values;

    formula_map_literal_node(parse_location location, formula_vector keys, formula_vector values) :
        formula_node(std::move(location)), keys(std::move(keys)), values(std::move(values)) {}

    void post_process(formula_post_process_context& context) override {
        for (auto &key: keys) {
            key->post_process(context);
        }

        for (auto &value: values) {
            value->post_process(context);
        }
    }

    datum evaluate(formula_evaluation_context& context) const override {
        tt_assert(keys.size() == values.size());

        datum::map_type r;
        for (size_t i = 0; i < keys.size(); i++) {
            ttlet &key = keys[i];
            ttlet &value = values[i];

            r[key->evaluate(context)] = value->evaluate(context);
        }
        return datum{std::move(r)};
    }

    std::string string() const noexcept override {
        tt_assert(keys.size() == values.size());

        std::string r = "{";
        for (size_t i = 0; i < keys.size(); i++) {
            ttlet &key = keys[i];
            ttlet &value = values[i];

            if (i > 0) {
                r += ", ";
            }
            r += key->string();
            r += ": ";
            r += value->string();
        }
        return r + "}";
    }
};

}