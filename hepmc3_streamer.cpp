// ROOT I/O streamer for HepMC3::GenEvent.
//
// GenEvent holds its particles and vertices through shared_ptrs, which ROOT
// cannot stream member-wise. When HEPMC3_ROOTIO is defined, HepMC3 declares a
// custom GenEvent::Streamer; it is normally implemented in libHepMC3rootIO,
// which the conda-forge build does not ship. This provides it, persisting the
// event through the flat HepMC3::GenEventData representation.

#include "HepMC3/Data/GenEventData.h"
#include "HepMC3/GenEvent.h"

#include "TBuffer.h"
#include "TClass.h"

void HepMC3::GenEvent::Streamer(TBuffer& b)
{
  static TClass* const data_class = TClass::GetClass<GenEventData>();
  GenEventData data;
  if (b.IsReading()) {
    b.ReadClassBuffer(data_class, &data);
    read_data(data);
  } else {
    write_data(data);
    b.WriteClassBuffer(data_class, &data);
  }
}
