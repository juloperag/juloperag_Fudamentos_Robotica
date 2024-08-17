import numpy as np
import matplotlib.pyplot as plt

# Constantes
E_F = 5.0  # Energía de Fermi (ajustar según el caso)
hbar = 1.0545718e-34  # Constante de Planck reducida
m = 9.10938356e-31  # Masa del electrón
a = 1.0  # Periodo de la red (ajustar según el caso)

# Rango de kx y ky
kx = np.linspace(-np.pi/a, np.pi/a, 500)
ky = np.linspace(-np.pi/a, np.pi/a, 500)
KX, KY = np.meshgrid(kx, ky)

# Energía E(kx, ky)
V00 = 4.0
Vpi0 = 0.3
V0pi = 0.3
E = V00 + Vpi0 * np.cos(np.pi * KX) + V0pi * np.cos(np.pi * KY)

# Condición para la superficie de Fermi
surface = (E_F - E) * 2 * m / hbar**2
surface = np.sqrt(surface)
surface = np.nan_to_num(surface)

# Graficar la superficie de Fermi
plt.contour(KX, KY, surface, levels=[0], colors='r')
plt.xlabel('$k_x$')
plt.ylabel('$k_y$')
plt.title('Superficie de Fermi')
plt.show()