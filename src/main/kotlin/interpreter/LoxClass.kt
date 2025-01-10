package org.zakat.interpreter

class LoxClass(
    internal val name: String,
    private val methods: Map<String, LoxFunction>,
): LoxCallable {
    override fun call(interpreter: Interpreter, args: MutableList<Any?>): Any? {
        val instance = LoxInstance(this)
        val initializer = findMethod("init")
        if (initializer != null) {
            initializer.bind(instance).call(interpreter, args)
        }
        return instance
    }

    override fun arity(): Int {
        val initializer = findMethod("init")
        return if (initializer == null) 0 else initializer.arity()
    }

    override fun toString(): String {
        return name
    }

    fun findMethod(name: String): LoxFunction? {
        return methods[name]
    }
}