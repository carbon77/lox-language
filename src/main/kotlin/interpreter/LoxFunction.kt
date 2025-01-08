package org.zakat.interpreter

import org.zakat.construct.Statement
import org.zakat.environment.Environment

class LoxFunction(
    private val declaration: Statement.Function,
    private val closure: Environment,
) : LoxCallable {
    override fun call(interpreter: Interpreter, args: MutableList<Any?>): Any? {
        val env = Environment(closure)
        for (i in declaration.params.indices) {
            env.define(declaration.params[i].lexeme, args[i])
        }

        try {
            interpreter.executeBlock(declaration.body, env)
        } catch (returnValue: Return) {
            return returnValue.value
        }
        return null
    }

    override fun arity(): Int {
        return declaration.params.size
    }

    override fun toString(): String {
        return "<fn ${declaration.name.lexeme}>"
    }
}