import agros2d
import pythonlab
from math import pi

agros2d.open_file(pythonlab.datadir('resources/examples/Examples/Magnetic Field/Magnetostatics/Switched Reluctance Machine.a2d'))

def get_energy(dphi, Jext):
    problem = agros2d.problem()
    magnetic = agros2d.field("magnetic")

    for coil in Jext:
        magnetic.modify_material("Copper {0}+".format(coil), {"magnetic_current_density_external_real" : +Jext[coil]})
        magnetic.modify_material("Copper {0}-".format(coil), {"magnetic_current_density_external_real" : -Jext[coil]})

    geometry = agros2d.geometry
    geometry.select_edges(range(64, 80))
    geometry.rotate_selection(0, 0, dphi)

    problem.solve()
    return magnetic.volume_integrals()['Wm']

def get_density(phi):
    Jext = {'A' : 0.0, 'B' : 0.0, 'C' : 0.0}
    J_EXT = 2e6

    if ((phi >= 60 and phi < 90) or
        (phi >= 150 and phi < 180)):
        Jext['A'] = J_EXT

    if ((phi >= 0 and phi < 30) or
        (phi >= 90 and phi < 120) or
        (phi == 180)):
        Jext['B'] = J_EXT

    if ((phi >= 30 and phi < 60) or
        (phi >= 120 and phi < 150)):
        Jext['C'] = J_EXT

    return Jext

step = 2.0
space = list()
density = list()
energy = list()
torque = list()

phi = 0.0
while phi <= 90.0:
    Jext = get_density(phi)
    Wm = get_energy(step * bool(phi), Jext)
    print('phi: {0}; Jext : {1}, Wm : {2}'.format(phi, Jext, Wm))

    space.append(phi)
    density.append(Jext)
    energy.append(Wm)
    phi = phi + step

for i in range(0, len(energy)):
    if (i != len(energy)-1):
        step = space[i+1] - space[i]
        torque.append(-(energy[i+1] - energy[i]) / (step/180.0 * pi))
    else:
        torque.append(torque[0])

    if (torque[-1] > 0.0):
        torque[-1] = 0.0

pythonlab.chart(space, energy, 'phi (deg.)', 'Wm (J)')
pythonlab.chart(space, torque, 'phi (deg.)', 'T (Nm)')