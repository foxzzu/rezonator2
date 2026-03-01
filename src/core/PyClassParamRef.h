#ifndef PY_CLASS_PARAM_REF_H
#define PY_CLASS_PARAM_REF_H

#include "PyUtils.h"
#include "Element.h"
#include "Parameters.h"

namespace PyClass::ParamRef {

const char *moduleName = nullptr;

PyTypeObject* type();
PyObject* make(Z::Parameter *param);

struct Self
{
    PyObject_HEAD
    Z::Parameter *param;
    std::shared_ptr<ElementEventsLocker> locker;
    std::shared_ptr<Z::ParamValueBackup> backup;
};

void dtor(Self *self)
{
    self->backup.reset();
    self->locker.reset();
    Py_TYPE(self)->tp_free((PyObject*)self);
}

PyObject* value(Self *self, PyObject *Py_UNUSED(args))
{
    return PyFloat_FromDouble(self->param->value().toSi());
}

PyObject* set_value(Self *self, PyObject *args)
{
    double val = qQNaN();
    if (!PyArg_Parse(args, "d", &val))
        return nullptr;
    CHECK_(!qIsNaN(val), ValueError, "invalid parameter value")
    Z::Value value(val, self->param->dim()->siUnit());
    self->param->setValue(value.toUnit(self->param->value().unit()));
    Py_RETURN_NONE;
}

PyObject* __enter__(Self* self, PyObject *Py_UNUSED(args))
{
    Py_INCREF(self);
    self->locker.reset(new ElementEventsLocker(self->param, "py.ParamRef.__enter__"));
    self->backup.reset(new Z::ParamValueBackup(self->param, "py.ParamRef.__enter__"));
    return (PyObject*)self;
}

PyObject* __exit__(Self* self, PyObject* args)
{
    // restore values _before_ unlocking events
    self->backup.reset();
    self->locker.reset();
    Py_RETURN_NONE;
}

PyTypeObject* type()
{
    static PyMethodDef methods[] = {
        { "value", (PyCFunction)value, METH_NOARGS, "Return parameter value (in SI units) by alias" },
        { "set_value", (PyCFunction)set_value, METH_O, "Set parameter value (in SI units) by alias" },
        {"__enter__", (PyCFunction)__enter__, METH_NOARGS,  "Context enter"},
        {"__exit__",  (PyCFunction)__exit__,  METH_VARARGS, "Context exit"},
        { NULL }
    };
    
    static QByteArray typeName = QString("%1.ParamRef").arg(moduleName).toUtf8();
    
    static PyTypeObject type = {
        .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = typeName.constData(),
        .tp_basicsize = sizeof(Self),
        .tp_itemsize = 0,
        .tp_dealloc = (destructor)dtor,
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_DISALLOW_INSTANTIATION,
        .tp_doc = PyDoc_STR("Parameter reference"),
        .tp_methods = methods,
    };

    return &type;
}

PyObject* make(Z::Parameter *param)
{
    MAKE_OBJECT
    if (obj) {
        obj->param = param;
    }
    return (PyObject*)obj;
}

} // PyClass::ParamRef

#endif // PY_CLASS_PARAM_REF_H