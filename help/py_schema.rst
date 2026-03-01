.. index:: single: schema (Python API)

Module: schema
==============

The ``schema`` module provides functions to access and manipulate the current optical schema.

Functions
---------

.. #######################################################################

.. index:: single: elem (Python API)
.. _py_method_schema_elem:

``elem(identifier)``
~~~~~~~~~~~~~~~~~~~~

Get an element by label or index.

Arguments:

- ``identifier`` - Element label (str) or 1-based index (int). Unlike regular Python lists, elements in schema are indexed starting from 1 to be consistent with how they are numbered in the :doc:`schema_elems`.

Returns:

- :doc:`Element <py_element>` - The requested element

Raises:

- ``KeyError`` - If element with given label is not found
- ``IndexError`` - If element with given index is not found
- ``TypeError`` - If identifier is neither string nor integer

Example:

.. code-block:: python

    import schema

    # Get element by label
    elem1 = schema.elem('Cr')

    # Get element by 1-based index
    elem2 = schema.elem(2)

.. #######################################################################

.. index:: single: elem_count (Python)

``elem_count()``
~~~~~~~~~~~~~~~~

Get the total number of elements in the schema.

Returns:

- int - Number of elements

Example:

.. code-block:: python

    import schema
    count = schema.elem_count()
    for i in range(count):
        elem = schema.elem(i+1)  # Elements are 1-based
        # Process element...

.. #######################################################################

.. index:: single: wavelength (Python)

``wavelength()``
~~~~~~~~~~~~~~~~

Get the current :doc:`wavelength <wavelen>`.

Returns:

- float - Wavelength in meters

Example:

.. code-block:: python

    import schema
    wl = schema.wavelength()
    print(f'Wavelength: {wl*1e9} nm')

.. #######################################################################

.. index:: single: param (Python)

``param(label)``
~~~~~~~~~~~~~~~~

Get a :doc:`global parameter <params_window>` value by label.

Arguments:

- ``label`` (str) - Parameter label

Returns:

- float - Parameter value in SI units, or None if not found

Example:

.. code-block:: python

    import schema
    p1_value = schema.param('P1')

.. #######################################################################

.. index:: single: set_param (Python)

``set_param(label, value)``
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Set a :doc:`global parameter <params_window>` value by label.

Arguments:

- ``label`` (str) - Parameter label
- ``value`` (float) - Parameter value in SI units

Raises:

- ``KeyError`` - If parameter is not found
- ``ValueError`` - If value is invalid

Example:

.. code-block:: python

    import schema
    schema.set_param('P1', 10e-3)


.. #######################################################################

.. index:: single: lock_param (Python)
.. _py_method_schema_lock_param:

``lock_param(label)``
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Lock element those are directly or indirectly depend on  a :doc:`global parameter <params_window>`.

Arguments:

- ``label`` (str) - Parameter label

Raises:

- ``KeyError`` - If parameter is not found

.. #######################################################################

.. index:: single: unlock_param (Python)
.. _py_method_schema_unlock_param:

``unlock_param(label)``
~~~~~~~~~~~~~~~~~~~~~~~

Unlock elements previously locked by the :ref:`lock_param <py_method_schema_lock_param>` method.

Arguments:

- ``label`` (str) - Parameter label

Raises:

- ``KeyError`` - If parameter is not found

.. note::

  Use ``schema.lock_param()`` and ``schema.unlock_param()`` methods when making temporary parameter changes (e.g., during :doc:`plot function <custom_plot>` calculation) to ensure proper restoration and also to prevent the UI from unnecessary updates. UI updates can significantly slow down the application when happen too often.

Example:

.. code-block:: python

    import schema
    schema.lock_param('P1')
    try:
      # Make temporary changes:
      for i in range(num_points):
        schema.set_param('P1', i)
        # Calculate something...
    finally:
      # Restore original values
      schema.unlock_param('P1')

.. #######################################################################

.. index:: single: param_ref (Python)
.. _py_method_schema_param_ref:

``param_ref(label)``
~~~~~~~~~~~~~~~~~~~~

Create a :doc:`ParamRef <py_param_ref>` object for more convenient implementation of the lock-unlock pattern in plotting functions.

Arguments:

- ``label`` (str) - Parameter label

Raises:

- ``KeyError`` - If parameter is not found

Example:

.. code-block:: python

    import schema
    with schema.param_ref('P1') as p:
      # Make temporary changes:
      for i in range(num_points):
        p.set_value(i)
        # Calculate something...
    #  Original value is automatically restored

.. #######################################################################

.. index:: single: is_sp (Python)
.. index:: single: is_sw (Python)
.. index:: single: is_rr (Python)

``is_sp()``, ``is_sw()``, ``is_rr()``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Check the schema :doc:`type <trip_type>`.

Returns:

- bool - True if schema is of the corresponding type

Example:

.. code-block:: python

    import schema
    if schema.is_sp():
        print('Single-pass system')
    elif schema.is_sw():
        print('Standing wave resonator')
    elif schema.is_rr():
        print('Ring resonator')

.. #######################################################################

.. index:: single: round_trip (Python)
.. _py_method_schema_round_trip:

``round_trip(ref=None, plane='T', inside=False)``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create a :ref:`round-trip <round_trip>` calculator for beam parameters.

Arguments:

- ``ref`` - Reference element (:doc:`Element <py_element>`, str label, int index, or None for last element)
- ``plane`` - :ref:`Work plane <work_plane>` ('T', 'S', :ref:`Z.PLANE_T <py_const_plane_t>`, or :ref:`Z.PLANE_S <py_const_plane_s>`)
- ``inside`` (bool) - If True, :ref:`split range elements <calc_round_trip_subrange>` at reference point

Returns:

- :doc:`RoundTrip <py_round_trip>` - Round trip calculator object

Raises:

- ``KeyError`` - If reference element is not found
- ``ValueError`` - If invalid plane name or element reference

Example:

.. code-block:: python

    import rezonator as Z
    import schema
    # Create round trip for element 'M2' in tangential plane
    rt_t = schema.round_trip(ref='M2', plane='T')
    # Create round trip with element split at midpoint
    elem = schema.elem(4)
    elem.offset = elem.axial_length / 2
    rt = schema.round_trip(ref=elem, inside=True, plane=Z.PLANE_T)

.. #######################################################################

.. seeAlso::

  - :doc:`py_api`
  - :doc:`py_global`
  - :ref:`Example - Basic Schema Access <py_example_basic_schema_access>`
  - :ref:`Example - Basic Element Manipulation <py_example_basic_elem_manipulation>`
