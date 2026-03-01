.. index:: single: ParamRef (Python)

Class: ParamRef
===============

Reference to a global or element's parameter optimized for multiple changes of the parameter value and restoring its original value, which is required for plotting functions.

Objects of this class cannot be constructed directly; instead, use the :ref:`schema.param_ref() <py_method_schema_param_ref>` or :ref:`Element.param_ref() <py_method_elem_param_ref>` functions to obtain them.

This class is intended to be used with Python's ``with`` statement. When execution enters the block, the original parameter value is remembered, and all elements directly or indirectly depending on the parameter get :ref:`locked <custom_plot_lock>` to prevent unnecessary UI changes while modifying the parameter value. After execution exits the block, the original parameter value gets restored and all locked elements are unlocked. This simplifies the plotting and does not make you remember to explicitly call for lock/unlock methods.

The typical scenario of usage it this

.. code-block:: python

    # Prepare arrays for store graph points
    graph_x = []
    graph_y = []
    # Prepare a round trip once,
    # no need to rebuild it for each point
    rt = schema.round_trip()
    with schema.param_ref('L_foc') as L_foc:
      for i in range(num_points):
        # Calculate the next argument
        x = L_foc_min + L_foc_step * i
        L_foc.set_value(x)
        # Calculate the next plot point
        y = rt.beam_radius()
        graph_x.append(x)
        graph_y.append(y)

See the ``global_params_py_set.rez`` example for to see it in action.

Methods
-------

.. index:: single: value (Python)
.. _py_method_param_ref_value:

``value()``
~~~~~~~~~~~

Return parameter value in SI units.

.. #######################################################################

.. index:: single: set_value (Python)
.. _py_method_param_ref_set_value:

``set_value(value)``
~~~~~~~~~~~~~~~~~~~~

Set parameter value.

Arguments:

- ``value`` (float) - Parameter value in SI units

Raises:

- ``ValueError`` - If value is invalid

.. seeAlso::

  - :doc:`custom_plot`
