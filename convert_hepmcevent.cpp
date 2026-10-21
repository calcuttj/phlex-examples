#include "convert_hepmcevent.hpp"

namespace examples {
  HepMC3::GenEventData convert_to_data(HepMC3::GenEvent const& input_event) {
    HepMC3::GenEventData output;
    input_event.write_data(output);
    return output;
  }

  HepMC3::GenEvent convert_from_data(HepMC3::GenEventData const& input_event) {
    HepMC3::GenEvent output;
    output.read_data(input_event);
    return output;
  }
} // namespace examples


