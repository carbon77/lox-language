package org.zakat.environment

import org.zakat.interpreter.RuntimeError
import org.zakat.lexer.Token

class Environment(val enclosing: Environment?) {
    constructor() : this(null)

    private val values = mutableMapOf<String, Any?>()

    fun define(name: String, value: Any?) {
        values[name] = value
    }

    fun assign(name: Token, value: Any?) {
        if (name.lexeme in values) {
            values[name.lexeme] = value
            return
        }

        if (enclosing != null) {
            enclosing.assign(name, value)
            return
        }

        throw RuntimeError(name, "Undefined variable '${name.lexeme}'.")
    }

    operator fun get(name: Token): Any? {
        if (name.lexeme in values) {
            return values[name.lexeme]
        }

        if (enclosing != null) return enclosing[name]

        throw RuntimeError(name, "Undefined variable '${name.lexeme}'.")
    }

    fun getAt(distance: Int, name: String): Any? {
        return ancestor(distance).values[name]
    }

    private fun ancestor(distance: Int): Environment {
        var env = this
        for (i in 0 until distance) {
            env = env.enclosing!!
        }
        return env
    }

    fun assignAt(distance: Int, name: Token, value: Any?) {
        ancestor(distance).values[name.lexeme] = value
    }
}