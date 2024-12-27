/*
 * This file was auto-generated by speedy-antlr-tool v{{__version__}}
 *  https://github.com/amykyta3/speedy-antlr-tool
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <cstring>
#include <any>

#include "antlr4-runtime.h"
#include "{{grammar_name}}Lexer.h"
#include "{{grammar_name}}Parser.h"
#include "speedy_antlr.h"

#include "sa_{{grammar_name|lower}}_translator.h"

{%- for rule_name in entry_rule_names %}
antlr4::tree::ParseTree* get_parse_tree_{{rule_name}}({{grammar_name}}Parser *parser) {return parser->{{rule_name}}();}
{%- endfor %}

antlr4::tree::ParseTree* get_parse_tree({{grammar_name}}Parser *parser, const char *entry_rule_name) {
    static std::map<std::string, antlr4::tree::ParseTree* (*)({{grammar_name}}Parser*)> table
    {
{%- for rule_name in entry_rule_names %}
        {"{{rule_name}}", &get_parse_tree_{{rule_name}}}
        {%- if not loop.last %},{% endif %}
{%- endfor %}
    };

    auto entry = table.find(entry_rule_name);
    if (entry != table.end()) {
        return (*(entry->second))(parser);
    } else {
        PyErr_SetString(PyExc_ValueError, "Invalid entry_rule_name");
        throw speedy_antlr::PythonException();
    }
}

/*
 * Python function prototype:
 *  do_parse(
 *      parser_cls:antlr4.Parser,
 *      stream:antlr4.InputStream,
 *      entry_rule_name:str,
 *      sa_err_listener:SA_ErrorListener,
 *      use_2_stage_parsing:bool
 *  )
 */
PyObject* do_parse(PyObject *self, PyObject *args) {
    PyObject *strdata = NULL;
    PyObject *result = NULL;
    PyObject *token_module = NULL;

    try {
        // Get args
        PyObject *parser_cls = NULL;
        PyObject *stream = NULL;
        const char *entry_rule_name = NULL;
        PyObject *sa_err_listener = NULL;
        PyObject *use_2_stage_parsing = NULL;
        if(!PyArg_ParseTuple(args,
            "OOsO:do_parse",
            &parser_cls, &stream, &entry_rule_name, &sa_err_listener, &use_2_stage_parsing
        )) {
            return NULL;
        }

        // Extract input stream's string
        char *cstrdata;
        Py_ssize_t bufsize;
        strdata = PyObject_GetAttrString(stream, "strdata");
        if(!strdata) throw speedy_antlr::PythonException();

        // PyUnicode_AsUTF8AndSize is not part of the stable ABI until python3.10
        // To maximize backwards compatibility, Working around by converting to
        // bytes, then to char instead
        PyObject *strdata_as_bytes;
        strdata_as_bytes = PyCodec_Encode(strdata, "utf-8", NULL);
        if(!strdata_as_bytes) throw speedy_antlr::PythonException();
        PyBytes_AsStringAndSize(strdata_as_bytes, &cstrdata, &bufsize);
        if(!cstrdata) throw speedy_antlr::PythonException();

        // Create an antlr InputStream object
        antlr4::ANTLRInputStream cpp_stream(cstrdata, bufsize);

        // in case error listener is overridden
        token_module = PyImport_ImportModule("antlr4.Token");
        if(!token_module) throw speedy_antlr::PythonException();
        speedy_antlr::Translator translator(parser_cls, stream);
        speedy_antlr::ErrorTranslatorListener err_listener(&translator, sa_err_listener);

        // Lex
        {{grammar_name}}Lexer lexer(&cpp_stream);
        if(sa_err_listener != Py_None){
            lexer.removeErrorListeners();
            lexer.addErrorListener(&err_listener);
        }
        antlr4::CommonTokenStream token_stream(&lexer);
        token_stream.fill();

        // Parse
        {{grammar_name}}Parser parser(&token_stream);
        if(sa_err_listener != Py_None){
            parser.removeErrorListeners();
            parser.addErrorListener(&err_listener);
        }

        if(PyObject_IsTrue(use_2_stage_parsing)) {
            // 2 Stage Parsing 
            // Stage 1: SLL mode 
            parser.getInterpreter<antlr4::atn::ParserATNSimulator>()->setPredictionMode(antlr4::atn::PredictionMode::SLL); 
            antlr4::tree::ParseTree *parse_tree = nullptr; 
            try { 
                parse_tree = get_parse_tree(&parser, entry_rule_name); // Attempt parsing in SLL mode 
            } catch (antlr4::ParseCancellationException &e) { 
                // Stage 2: Fallback to LL mode 
                token_stream.reset(); 
                parser.reset(); 
                parser.getInterpreter<antlr4::atn::ParserATNSimulator>()->setPredictionMode(antlr4::atn::PredictionMode::LL); 
                parse_tree = get_parse_tree(&parser, entry_rule_name); // Retry parsing in LL mode 
            }
        } else {
            antlr4::tree::ParseTree *parse_tree;
            parse_tree = get_parse_tree(&parser, entry_rule_name);
        }

        // Translate Parse tree to Python
        SA_{{grammar_name}}Translator visitor(&translator);
        result = std::any_cast<PyObject *>(visitor.visit(parse_tree));

        // Clean up data
        Py_XDECREF(token_module);
        Py_XDECREF(strdata_as_bytes);
        Py_XDECREF(strdata);

        return result;

    } catch(speedy_antlr::PythonException &e) {
        Py_XDECREF(token_module);
        Py_XDECREF(strdata);
        Py_XDECREF(result);

        // Python exception already has error indicator set
        return NULL;
    } catch(...) {
        Py_XDECREF(token_module);
        Py_XDECREF(strdata);
        Py_XDECREF(result);

        // An internal C++ exception was thrown.
        // Set error indicator to a generic runtime error
        PyErr_SetString(PyExc_RuntimeError, "Internal error");
        return NULL;
    }
}


extern "C" {

    static PyObject* c_do_parse(PyObject *self, PyObject *args) {
        return do_parse(self, args);
    }

    static PyMethodDef methods[] = {
        {
            "do_parse",  c_do_parse, METH_VARARGS,
            "Run parser"
        },
        {NULL, NULL, 0, NULL} /* Sentinel */
    };

    static struct PyModuleDef module = {
        PyModuleDef_HEAD_INIT,
        "sa_{{grammar_name|lower}}_cpp_parser",   /* name of module */
        NULL, /* module documentation, may be NULL */
        -1,       /* size of per-interpreter state of the module,
                    or -1 if the module keeps state in global variables. */
        methods
    };
}


PyMODINIT_FUNC
PyInit_sa_{{grammar_name|lower}}_cpp_parser(void) {
    PyObject *m = PyModule_Create(&module);
    return m;
}
