"""A simple event generator written in Python.

Python port of simple_generator_hepmc.cpp.  Phlex's Python plugin can only
pass numbers and 1-D numeric arrays to the rest of the workflow, so instead of
a HepMC3::GenEvent this provider returns a flat event table (see
genevent_table.py).  The C++ transform genevent_from_table turns that table
into a HepMC3::GenEvent.
"""

import math

import numpy as np
import numpy.typing as npt

import phlex_config
from genevent_table import GenEventTable
from phlex import Variant


def generate(
    event_number: int, position: tuple[float, float, float], muon_angle: float
) -> npt.NDArray[np.float64]:
    """Generate nu + n -> mu + p with a 1 GeV neutrino along z.

    Args:
        event_number (int): Event number stored in the table.
        position (tuple): Vertex position (x, y, z) in cm.
        muon_angle (float): Muon angle to the neutrino direction, in degrees.

    Returns:
        numpy.ndarray: The event table described in genevent_table.py.
    """
    # Assume a 1 GeV neutrino (travelling only in z) strikes an at-rest neutron
    # and produces a muon at a fixed angle: nu + n -> mu + p.
    # Energy-momentum conservation fixes the muon energy once the angle is chosen.
    enu = 1.0  # GeV
    mn = 0.939565
    mp = 0.938272
    mmu = 0.105658
    thetamu = math.radians(muon_angle)

    # Eliminating the proton four-momentum gives A*pmu - B*Emu + C = 0 with
    # pmu = sqrt(Emu^2 - mmu^2). Squaring yields a quadratic in Emu.
    a = 2.0 * enu * math.cos(thetamu)
    b = 2.0 * (enu + mn)
    c = mn * mn - mp * mp + mmu * mmu + 2.0 * enu * mn
    d = b * b - a * a
    # Using the signed A (cos(theta) < 0 above 90 degrees) picks the physical root at
    # any angle: A*pmu = B*Emu - C must have A's sign, which the other root violates.
    emu = (b * c + a * math.sqrt(c * c - mmu * mmu * d)) / d

    pmu = math.sqrt(emu * emu - mmu * mmu)
    pmu_z = math.cos(thetamu) * pmu
    pmu_t = math.sin(thetamu) * pmu

    # enu == pnu == p_proton + p_muon
    pp_z = enu - pmu_z

    table = GenEventTable(event_number)
    vtx = table.add_vertex(*position)
    table.add_particle(vtx, 13, 1, (0.0, pmu_t, pmu_z, emu), mmu)
    # Proton balances out pmu_T (transverse momentum)
    ep = math.sqrt(pp_z * pp_z + pmu_t * pmu_t + mp * mp)
    table.add_particle(vtx, 2212, 1, (0.0, -pmu_t, pp_z, ep), mp)
    return table.to_array()


def PHLEX_REGISTER_PROVIDERS(s, config):
    """Register the generator as a provider of the particle table.

    Args:
        s (internal): Phlex source representation.
        config (internal): Phlex configuration representation.

    Returns:
        None
    """
    position = phlex_config.xyz(phlex_config.get(config, "position", "0 0 0"))
    muon_angle = float(phlex_config.get(config, "muon_angle", 100.0))

    def provide(di):
        return generate(di.number(), position, muon_angle)

    f = Variant(
        provide,
        {"di": "data_cell_index", "return": npt.NDArray[np.float64]},
        "generate",
    )
    s.provide(
        f,
        {
            "creator": phlex_config.get(config, "creator", "pygen"),
            "layer": config["layer"],
            "suffix": phlex_config.get(config, "suffix", "particles"),
        },
    )
