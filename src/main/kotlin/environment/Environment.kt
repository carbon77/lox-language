package org.zakat.environment

import org.zakat.interpreter.RuntimeError
import org.zakat.lexer.Token

class Environment {
    private val values = mutableMapOf<String, Any?>()

    operator fun set(name: String, value: Any?) {
        values[name] = value
    }

    operator fun get(name: Token): Any? {
        if (name.lexeme in values) {
            return values[name.lexeme]
        }

        throw RuntimeError(name, "Undefined variable '${name.lexeme}'.")
    }
}