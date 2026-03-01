#ifndef PY_MODULE_SCHEMA_H
#define PY_MODULE_SCHEMA_H

#include "PyUtils.h"
#include "PyClassElement.h"
#include "PyClassParamRef.h"
#include "PyClassRoundTrip.h"
#include "Schema.h"
#include "../math/BeamCalculator.h"
#include "../math/FunctionUtils.h"

namespace PyModule::Schema {

const char *moduleName = "schema";

struct ParamLocker
{
    ParamLocker(Z::Parameter *param): alias(param->alias()), param(param)
    {
        Elements elems;
        ElementEventsLocker::collectElems(param, elems);
        for (auto elem : std::as_const(elems))
        {
            if (elem->params().indexOf(param) >= 0)
                owner = elem;
            lockers.insert(elem, new ElementEventsLocker(elem, "py.schema.lock_param"));
        }
        backup = new Z::ParamValueBackup(param, "py.schema.lock_param");
    }
    
    ~ParamLocker()
    {
        auto schema = PyGlobal::schema;
        auto globalParams = schema->globalParamsAsElem();
    
        // Python doesn't unload modules after a code has been run.
        // So one could write in a code window a code like
        // `schema.lock_param('P1')` without successive `schema.unlock_param('P1')`.
        // Imagine then they do some manipulations whith elements or global parameters
        // then adds to the code a call `schema.unlock_param('P1')`.
        // The app will crash if some elements
        // pointers to which are cached in this locker
        // have been deleted.
        // So we have to do some precautions to ensure if all elements are still exist
        // and skip unlocking/restoring if they are not
    
        for (auto it = lockers.begin(); it != lockers.end(); it++)
            if (schema->indexOf(it.key()) < 0 && it.key() != globalParams)
            {
                it.value()->cancel();
                qWarning() << "py.schema.unlock_param"<< alias << 
                    ": one of locked elements has been already deleted, skip unlocking";
            }
        
        // Parameters that can be used in code always have some owners.
        // Theys are either element parameres 
        // or globals parameters, which are also owned by a special element.
        // So we can check if the parameters still exist, but checking its owner's parameters list
        if (schema->indexOf(owner) < 0 && owner != globalParams)
        {
            backup->cancel();
            qWarning() << "py.schema.unlock_param"<< alias << 
                ": parameter owner has been deleted, skip restoring";
        }
        else if (owner->params().indexOf(param) < 0)
        {
            backup->cancel();
            qWarning() << "py.schema.unlock_param"<< alias << 
                ": parameter has been deleted, skip restoring";
        }

        // restore values _before_ unlocking events
        delete backup;
        qDeleteAll(lockers);
    }

    QString alias;
    Z::Parameter *param;
    Element *owner = nullptr;
    QHash<Element*, ElementEventsLocker*> lockers;
    Z::ParamValueBackup *backup;
};

struct State
{
    QHash<QString, ParamLocker*> *lockedParams;
};

PyObject* wavelength(PyObject* Py_UNUSED(self), PyObject* Py_UNUSED(args))
{
    CHECK_SCHEMA
    return PyFloat_FromDouble(SCHEMA->wavelength().value().toSi());
}

PyObject* param(PyObject* Py_UNUSED(self), PyObject* args)
{
    CHECK_SCHEMA
    const char *alias;
    if (!PyArg_Parse(args, "s", &alias))
        return nullptr;
    auto p = SCHEMA->param(alias);
    if (!p)
        Py_RETURN_NONE;
    return PyFloat_FromDouble(p->value().toSi());
}

PyObject* set_param(PyObject* Py_UNUSED(self), PyObject* args)
{
    char *alias;
    double val = qQNaN();
    if (!PyArg_ParseTuple(args, "s|d", &alias, &val))
        return nullptr;
    auto param = SCHEMA->param(alias);
    CHECK_(param, KeyError, "parameter not found")
    CHECK_(!qIsNaN(val), ValueError, "invalid parameter value")
    Z::Value value(val, param->dim()->siUnit());
    param->setValue(value.toUnit(param->value().unit()));
    Py_RETURN_NONE;
}

PyObject* lock_param(PyObject* self, PyObject* args)
{
    CHECK_SCHEMA
    const char *alias;
    if (!PyArg_Parse(args, "s", &alias))
        return nullptr;
    auto param = SCHEMA->param(alias);
    CHECK_(param, KeyError, "parameter not found")
    auto state = (State*)PyModule_GetState(self);
    if (!state)
        return nullptr;
    if (!state->lockedParams)
        state->lockedParams = new QHash<QString, ParamLocker*>;
    state->lockedParams->insert(param->alias(), new ParamLocker(param));
    Py_RETURN_NONE;
}

PyObject* unlock_param(PyObject* self, PyObject* args)
{
    CHECK_SCHEMA
    const char *alias;
    if (!PyArg_Parse(args, "s", &alias))
        return nullptr;
    auto state = (State*)PyModule_GetState(self);
    if (!state)
        return nullptr;
    QString paramAlias(alias);
    if (!state->lockedParams || !state->lockedParams->contains(paramAlias))
    {
        qWarning() << "py.schema.unlock_param: try to unlock not locked parameter" << paramAlias << "ignored";
        Py_RETURN_NONE;
    }
    auto param = SCHEMA->param(alias);
    CHECK_(param, KeyError, "parameter not found")
    delete state->lockedParams->take(paramAlias);
    Py_RETURN_NONE;
}

PyObject* param_ref(PyObject* Py_UNUSED(self), PyObject* args)
{
    CHECK_SCHEMA
    const char *alias;
    if (!PyArg_Parse(args, "s", &alias))
        return nullptr;
    auto param = SCHEMA->param(alias);
    CHECK_(param, KeyError, "parameter not found")
    return PyClass::ParamRef::make(param);
}

PyObject* elem(PyObject* Py_UNUSED(self), PyObject* arg)
{
    CHECK_SCHEMA
    ::Element *elem = nullptr;
    if (PyLong_Check(arg)) {
        auto index = PyLong_AsLong(arg);
        // For python code elemens are numbered 1-based
        // as they are shown in the elements table
        elem = SCHEMA->element(index-1);
        CHECK_(elem, IndexError, "element not found")
    } else if (PyUnicode_Check(arg)) {
        auto label = QString::fromUtf8(PyUnicode_AsUTF8(arg));
        elem = SCHEMA->element(label);
        CHECK_(elem, KeyError, "element not found")
    } else
        CHECK_(false, TypeError, "unsupported type of argument, integer or string expected")
    return PyClass::Element::make(elem);
}

PyObject* elem_count(PyObject *Py_UNUSED(self), PyObject *Py_UNUSED(args))
{
    CHECK_SCHEMA
    return PyLong_FromSize_t(SCHEMA->elements().size());
}

PyObject* is_sp(PyObject *Py_UNUSED(self), PyObject *Py_UNUSED(args))
{
    CHECK_SCHEMA
    return PyBool_FromLong(SCHEMA->isSP());
}

PyObject* is_sw(PyObject *Py_UNUSED(self), PyObject *Py_UNUSED(args))
{
    CHECK_SCHEMA
    return PyBool_FromLong(SCHEMA->isSW());
}

PyObject* is_rr(PyObject *Py_UNUSED(self), PyObject *Py_UNUSED(args))
{
    CHECK_SCHEMA
    return PyBool_FromLong(SCHEMA->isRR());
}

PyObject* round_trip(PyObject *Py_UNUSED(self), PyObject *args, PyObject *kwargs)
{
    CHECK_SCHEMA
    if (SCHEMA->activeCount() == 0) {
        PyErr_SetString(PyExc_KeyError, "there are no active elements in the schema");
        return nullptr;
    }
    
    // For python code elemens are numbered 1-based 
    // as they are shown in the elements table
    #define PARSE_ARG_REF \
        if (PyLong_Check(arg)) { \
            auto index = PyLong_AsLong(arg); \
            refElem = SCHEMA->element(index-1); \
            CHECK_(refElem, IndexError, "reference element not found") \
        } else if (PyUnicode_Check(arg)) { \
            auto label = QString::fromUtf8(PyUnicode_AsUTF8(arg)); \
            refElem = SCHEMA->element(label); \
            CHECK_(refElem, KeyError, "reference element not found") \
        } else if (PyObject_TypeCheck(arg, PyClass::Element::type())) { \
            refElem = ((PyClass::Element::Self*)arg)->elem; \
            CHECK_(refElem, ValueError, "element reference is null") \
            CHECK_(SCHEMA->elements().contains(refElem), ValueError, "reference element not found") \
        } else { \
            CHECK_(false, TypeError, "wrong type of the 'ref' arg, integer or string or Element expected"); \
        }
        
    #define PARSE_ARG_PLANE \
        if (PyUnicode_Check(arg)) { \
            auto plane = QString::fromUtf8(PyUnicode_AsUTF8(arg)).toUpper(); \
            if (plane == Z::planeName(Z::T)) \
                workPlane = Z::T; \
            else if (plane == Z::planeName(Z::S)) \
                workPlane = Z::S; \
            else \
                CHECK_(false, ValueError, "wrong work plane name, T or S expected") \
        } else if (PyLong_Check(arg)) { \
            auto plane = PyLong_AsLong(arg); \
            CHECK_(plane == Z::WorkPlane::T || plane == Z::WorkPlane::S, ValueError, \
                "unexpected value of the 'plane' arg, expected one of Z.PLANE_T or Z.PLANE_S") \
            workPlane = (Z::WorkPlane)plane; \
        } else { \
            CHECK_(false, TypeError, "wrong type of the 'plane' arg, string or integer expected") \
        }
        
    std::optional<bool> splitRange;
    std::optional<Z::WorkPlane> workPlane;
    ::Element *refElem = nullptr;
    if (args) {
        auto argCount = PyTuple_Size(args);
        if (argCount < 0)
            return nullptr;
        if (argCount > 0) {
            auto arg = PyTuple_GetItem(args, 0);
            if (!arg)
                return nullptr;
            if (arg != Py_None) {
                PARSE_ARG_REF
            }
        }
        if (argCount > 1) {
            auto arg = PyTuple_GetItem(args, 1);
            if (!arg)
                return nullptr;
            PARSE_ARG_PLANE
        }
        if (argCount > 2) {
            auto arg = PyTuple_GetItem(args, 2);
            if (!arg)
                return nullptr;
            splitRange = Py_IsTrue(arg);
        }
        CHECK_(argCount <= 3, TypeError, "wrong args count, 0..3 args expected")
    }
    if (kwargs) {
        if (auto arg = PyDict_GetItemString(kwargs, "ref"); arg) {
            CHECK_(!refElem, TypeError, "multiple values for argument 'ref'")
            PARSE_ARG_REF
        }
        if (auto arg = PyDict_GetItemString(kwargs, "plane"); arg) {
            CHECK_(!workPlane, TypeError, "multiple values for argument 'plane'")
            PARSE_ARG_PLANE
        }
        if (auto arg = PyDict_GetItemString(kwargs, "inside"); arg) {
            CHECK_(!splitRange, TypeError, "multiple values for argument 'inside'")
            splitRange = Py_IsTrue(arg);
        }
    }
    if (!refElem)
        refElem = SCHEMA->elements().last();
    auto beamCalc = new BeamCalculator(SCHEMA);
    if (!beamCalc->ok()) {
        PyErr_SetQString(PyExc_AssertionError, beamCalc->error());
        delete beamCalc;
        return nullptr;
    }
    if (!workPlane)
        workPlane = Z::WorkPlane::T;
    if (!splitRange)
        splitRange = false;
    beamCalc->calcRoundTrip(refElem, *splitRange, "py.schema.round_trip()");
    beamCalc->setPlane(*workPlane);
    beamCalc->setIor(FunctionUtils::ior(SCHEMA, refElem, *splitRange));
    return PyClass::RoundTrip::make(beamCalc, true);
    
    #undef PARSE_ARG_REF
    #undef PARSE_ARG_PLANE
}

int on_exec(PyObject *module)
{
    PyGlobal::SchemaError = PyErr_NewException("schema.error", NULL, NULL);
    if (PyModule_AddObjectRef(module, "SchemaError", PyGlobal::SchemaError) < 0)
        STOP_MODULE_INIT
    
    qDebug() << "Module executed:" << moduleName;
    return 0;
}

void on_free(PyObject *module)
{
    auto state = (State*)PyModule_GetState(module);
    if (state && state->lockedParams)
    {
        // Ideally there should be parameters unlocking, but I don't see that Python calls this method.
        // The module stays in memory all the time and is not freed even when its code gets replaced.
        
        delete state->lockedParams;
    }
}

PyMethodDef methods[] = {
    { "elem", elem, METH_O, "Return element by label or number" },
    { "elem_count", elem_count, METH_NOARGS, "Return number of elements in schema" },
    { "is_sp", is_sp, METH_NOARGS, "If schema is single pass system" },
    { "is_sw", is_sw, METH_NOARGS, "If schema is standing wave rezonator" },
    { "is_rr", is_rr, METH_NOARGS, "If schema is ring rezonator" },
    { "param", param, METH_O, "Return value of global parameter (in SI units)." },
    { "set_param", set_param, METH_VARARGS, "Set value of global parameter (in SI units)." },
    { "lock_param", lock_param, METH_O, "Disable events for elements driven by this parameter" },
    { "unlock_param", unlock_param, METH_O, "Enable events for elements driven by this parameter" },
    { "param_ref", param_ref, METH_O, "Return reference to global parameter." },
    { "wavelength", wavelength, METH_NOARGS, "Return current wavelength (in m)." },
    { "round_trip", (PyCFunction)round_trip, METH_VARARGS | METH_KEYWORDS,
        "Return a RoundTrip object that can be used for basic calculations." },
    { NULL, NULL, 0, NULL }
};

PyModuleDef_Slot slotes[] = {
    { Py_mod_exec, (void*)on_exec },
    { Py_mod_multiple_interpreters, Py_MOD_MULTIPLE_INTERPRETERS_NOT_SUPPORTED },
    { 0, NULL }
};

PyModuleDef module = {
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = moduleName,
    .m_size = sizeof(State),
    .m_methods = methods,
    .m_slots = slotes,
    .m_free = (freefunc)on_free,
};

PyObject* init()
{
    return PyModuleDef_Init(&module);
}

} // namespace PyModule::Schema

#endif // PY_MODULE_SCHEMA_H
