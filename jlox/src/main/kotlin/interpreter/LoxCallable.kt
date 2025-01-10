package org.zakat.interpreter

interface LoxCallable {
    fun call(interpreter: Interpreter, args: MutableList<Any?>): Any?
    fun arity(): Int
}
