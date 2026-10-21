#include "phlex/module.hpp"
#include "convert_hepmcevent.hpp"

PHLEX_REGISTER_ALGORITHMS(m, config)
{
  using namespace phlex;

  auto const input_layer = config.get<std::string>("input_layer");

  m.transform("convert_hepmcevent_from_data", examples::convert_from_data, concurrency::unlimited)
    .input_family(product_selector{.creator = "convert_hepmcevent_to_data", .layer = input_layer, .suffix="converted"})
    .output_product_suffixes("genevent");

}
