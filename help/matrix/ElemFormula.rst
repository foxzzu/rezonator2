.. _elem_formula:
.. index:: single: formula (element)

Formula Element
===============

The formula element allows for the design of arbitrary optical elements where ray matrices are calculated by a user-given :ref:`Python <py_api>` code.

When a formula element is added to the schema, use the :doc:`../elem_opers_formula` command for changing the element’s code. The code must provide a function ``calc_matrix`` returning a dictionary of element matrices. The dictionary should contain the Mt and Ms items for tangential and sagittal matrices, respectively. |rezonator| calls this function when it wants to recompute the element’s ray matrices and passes a reference of the target element. The code can query the element’s parameters and use them in calculations. Parameters should be created as :ref:`custom parameters <custom_params>`.

Here is an example of a simple formula element representing a thin lens. This element works the same as the built-in :ref:`Thin lens <elem_thin_lens>` element but allows for changing the lens dioptric power rather than its focal range.

.. code-block:: python

  from math import cos
  from rezonator import Element, Matrix

  def calc_matrix(elem: Element):
    P = elem.param('P', 1)
    a = elem.param('alpha', 0)
    return {
      'Mt': Matrix(1, 0, -P/cos(a), 1),
      'Ms': Matrix(1, 0, -P*cos(a), 1),
    }

When you :ref:`copy <elem_opers_copy>` such an element or :ref:`save <elem_opers_save_custom>` it to the :doc:`../elem_library`, the formula code and custom parameter are also duplicated, making an independent copy that can be changed without affecting the original element.

.. seeAlso::

    - :doc:`../elem_matrs`
    - :doc:`../catalog`
    - :doc:`../elem_props`
    - :doc:`../custom_params`
    - :doc:`../custom_funcs`
