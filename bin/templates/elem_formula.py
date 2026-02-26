'''
Dioptric lens

↑ This is a human readable element type name.

This element works the same as the built-in "Thin lens" element
but allows for changing the lens's dioptric power rather than its focal range.
'''

from math import cos
from rezonator import Element, Matrix

def calc_matrix(elem: Element):
  '''
  This function is automatically called when
  reZonator wants to recalculate element's ray matrices.
  '''

  # To make this element fully-functional
  # we need to add two custom parameters:
  # for power and angle.
  P = elem.param('P', 1)
  a = elem.param('alpha', 0)

  A = 1
  B = 0
  Ct = -P / cos(a)
  Cs = -P * cos(a)
  D = 1

  # The function should return a dictionary
  # containing matrices for T and S planes
  return {
    'Mt': Matrix(A, B, Ct, D),
    'Ms': Matrix(A, B, Cs, D),
  }
