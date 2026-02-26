.. _elem_opers_formula:

Edit Formula
============

:menuSelection:`Element --> Edit Formula`

The command is active only for the :doc:`matrix/ElemFormula`. The command opens a code editor window for changing the element's matrix calculation code. Unless applied to the element, the changed code only exits inside the window what, as stated in the window's header. If you close the window, all the changes will be lost. Use the :guiLabel:`Apply` button on the window's toolbar or the menu command :menuSelection:`Formula --> Apply Formula` to store the code in the source element and recalculate its matrices.

The window does not allow for modifying the element's parameters that are used inside the code. For this, use the :doc:`elem_props` dialog and its functionality for editing :doc:`custom_params`.

  .. image:: img/elem_opers_formula.png

.. seeAlso::

    - :doc:`elem_opers`
    - :doc:`custom_funcs`
