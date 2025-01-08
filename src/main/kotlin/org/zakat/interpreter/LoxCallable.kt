package org.zakat.org.zakat.interpreter

import org.zakat.interpreter.Interpreter

interface LoxCallable {
    fun call(interpreter: Interpreter, args: MutableList<Any?>): Any?
    fun arity(): Int
}
