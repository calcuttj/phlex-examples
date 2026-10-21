#ifndef CONVERT_HEPMCEVENT_HPP
#define CONVERT_HEPMCEVENT_HPP

#include <HepMC3/GenEvent.h>
#include <HepMC3/Data/GenEventData.h>


namespace examples {
  HepMC3::GenEventData convert_to_data(HepMC3::GenEvent const& input_event);
  HepMC3::GenEvent convert_from_data(HepMC3::GenEventData const& input_event);
} // namespace examples

#endif // CONVERT_HEPMCEVENT_HPP