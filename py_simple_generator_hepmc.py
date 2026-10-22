"""A simple event generator written in Python.

Python port of simple_generator_hepmc.cpp.  Phlex's Python plugin can only
pass numbers and 1-D numeric arrays to the rest of the workflow, so instead of
a HepMC3::GenEvent this provider returns a flat particle table.  The C++
transform genevent_from_table turns that table into a HepMC3::GenEvent.

Table layout (float64, GeV and cm), one vertex with outgoing particles only:

    [event_number, x, y, z, t,  then per particle: pdg, status, px, py, pz, E, mass]
"""

import math

import numpy as np
import numpy.typing as npt

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
        numpy.ndarray: The particle table described in the module docstring.
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
    # The "+" root is physical; the "-" root fails B*Emu - C >= 0 (spurious from squaring).
    emu = (b * c + a * math.sqrt(c * c - mmu * mmu * d)) / d

    pmu = math.sqrt(emu * emu - mmu * mmu)
    pmu_z = math.cos(thetamu) * pmu
    pmu_t = math.sin(thetamu) * pmu

    # enu == pnu == p_proton + p_muon
    pp_z = enu - pmu_z

    x, y, z = position
    header = [event_number, x, y, z, 0.0]
    # pdg, status, px, py, pz, E, mass
    muon = [13, 1, 0.0, pmu_t, pmu_z, emu, mmu]
    # Proton balances out pmu_T (transverse momentum)
    ep = math.sqrt(pp_z * pp_z + pmu_t * pmu_t + mp * mp)
    proton = [2212, 1, 0.0, -pmu_t, pp_z, ep, mp]
    return np.asarray(header + muon + proton, dtype=np.float64)


def PHLEX_REGISTER_PROVIDERS(s, config):
    """Register the generator as a provider of the particle table.

    Args:
        s (internal): Phlex source representation.
        config (internal): Phlex configuration representation.

    Returns:
        None
    """
    position = tuple(float(v) for v in config.get("position", (0.0, 0.0, 0.0)))
    muon_angle = float(config.get("muon_angle", 100.0))

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
            "creator": config.get("creator", "pygen"),
            "layer": config["layer"],
            "suffix": config.get("suffix", "particles"),
        },
    )
