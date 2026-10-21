#include "phlex/module.hpp"
#include "convert_hepmcevent.hpp"

PHLEX_REGISTER_ALGORITHMS(m, config)
{
  using namespace phlex;

  auto const input_layer = config.get<std::string>("input_layer");

  m.transform("convert_hepmcevent_to_data", examples::convert_to_data, concurrency::unlimited)
    .input_family(product_selector{.creator = "input", .layer = input_layer, .suffix="genevent"})
    .output_product_suffixes("converted");

}

