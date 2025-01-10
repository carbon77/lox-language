package org.zakat.interpreter

import org.zakat.construct.Statement
import org.zakat.environment.Environment

class LoxFunction(
    private val declaration: Statement.Function,
    private val closure: Environment,
    private val isInitializer: Boolean,
) : LoxCallable {

    override fun call(interpreter: Interpreter, args: MutableList<Any?>): Any? {
        val env = Environment(closure)
        for (i in declaration.params.indices) {
            env.define(declaration.params[i].lexeme, args[i])
        }

        try {
            interpreter.executeBlock(declaration.body, env)
        } catch (returnValue: Return) {
            if (isInitializer) return closure.getAt(0, "this")
            return returnValue.value
        }

        if (isInitializer) return closure.getAt(0, "this")
        return null
    }

    override fun arity(): Int {
        return declaration.params.size
    }

    override fun toString(): String {
        return "<fn ${declaration.name.lexeme}>"
    }

    fun bind(instance: LoxInstance): LoxFunction {
        val env = Environment(closure)
        env.define("this", instance)
        return LoxFunction(declaration, env, isInitializer)
    }
}