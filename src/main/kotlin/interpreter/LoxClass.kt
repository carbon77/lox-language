package org.zakat.interpreter

class LoxClass(
    internal val name: String,
): LoxCallable {
    override fun call(interpreter: Interpreter, args: MutableList<Any?>): Any? {
        val instance = LoxInstance(this)
        return instance
    }

    override fun arity(): Int {
        return 0;
    }

    override fun toString(): String {
        return name
    }
}