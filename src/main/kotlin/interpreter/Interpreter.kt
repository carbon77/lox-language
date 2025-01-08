package org.zakat.interpreter

import org.zakat.Lox
import org.zakat.construct.Expr
import org.zakat.construct.ExpressionVisitor
import org.zakat.construct.Statement
import org.zakat.construct.StatementVisitor
import org.zakat.environment.Environment
import org.zakat.lexer.Token
import org.zakat.lexer.TokenType

class Interpreter : ExpressionVisitor<Any?>, StatementVisitor {
    val globals = Environment()
    private var environment = globals

    init {
        globals.define("clock", object : LoxCallable {
            override fun arity() = 0
            override fun toString() = "<native fn>"
            override fun call(interpreter: Interpreter, args: MutableList<Any?>): Double {
                return System.currentTimeMillis() / 1000.0
            }
        })
    }

    fun interpret(statements: List<Statement?>) {
        try {
            for (statement in statements) {
                execute(statement)
            }
        } catch (e: RuntimeError) {
            Lox.runtimeError(e)
        }
    }

    private fun execute(statement: Statement?) {
        statement?.accept(this)
    }

    override fun visitBinaryExpression(expr: Expr.Binary): Any? {
        val left = evaluate(expr.left)
        val right = evaluate(expr.right)

        when (expr.operator.type) {
            // Check numeric operators
            TokenType.MINUS, TokenType.SLASH, TokenType.STAR,
            TokenType.GREATER, TokenType.GREATER_EQUAL,
            TokenType.LESS, TokenType.LESS_EQUAL ->
                checkNumberOperands(expr.operator, left, right)

            else -> {}
        }

        return when (expr.operator.type) {
            TokenType.MINUS -> left as Double - right as Double
            TokenType.SLASH -> {
                if (right is Double && right == 0.0)
                    throw RuntimeError(expr.operator, "Division by zero")
                left as Double / right as Double
            }

            TokenType.STAR -> left as Double * right as Double
            TokenType.GREATER -> left as Double > right as Double
            TokenType.GREATER_EQUAL -> left as Double >= right as Double
            TokenType.LESS -> (left as Double) < (right as Double)
            TokenType.LESS_EQUAL -> left as Double <= right as Double
            TokenType.EQUAL_EQUAL -> isEqual(left, right)
            TokenType.BANG_EQUAL -> !isEqual(left, right)
            TokenType.PLUS -> when {
                left is Double && right is Double -> left + right
                left is String && right is String -> left + right
                else -> RuntimeError(
                    expr.operator,
                    "Operands must be two numbers or two strings."
                )
            }

            else -> null
        }
    }

    override fun visitGroupingExpression(expr: Expr.Grouping): Any? {
        return evaluate(expr.expr)
    }

    override fun visitLiteralExpression(expr: Expr.Literal): Any? {
        return expr.value
    }

    override fun visitUnaryExpression(expr: Expr.Unary): Any? {
        val right: Any? = evaluate(expr.right)

        return when (expr.operator.type) {
            TokenType.MINUS -> {
                checkNumberOperand(expr.operator, right)
                -(right as Double)
            }

            TokenType.BANG -> !isTruthy(right)
            else -> null
        }
    }

    override fun visitVariableExpression(expr: Expr.Variable): Any? {
        return environment[expr.name]
    }

    override fun visitAssignExpression(expr: Expr.Assign): Any? {
        val value = evaluate(expr.value)
        environment.assign(expr.name, value)
        return value
    }

    override fun visitLogicalExpression(expr: Expr.Logical): Any? {
        val left = evaluate(expr.left)

        if (expr.operator.type == TokenType.OR) {
            if (isTruthy(left)) return left
        } else {
            if (!isTruthy(left)) return left
        }

        return evaluate(expr.right)
    }

    override fun visitCallExpression(expr: Expr.Call): Any? {
        val callee = evaluate(expr.callee)

        val args = mutableListOf<Any?>()
        for (arg in expr.arguments) {
            args.add(evaluate(arg))
        }

        if (callee !is LoxCallable) {
            throw RuntimeError(expr.paren,
                "Can only call functions and classes.")
        }

        if (args.size != callee.arity()) {
            throw RuntimeError(expr.paren,
                "Exprected ${callee.arity()} arguments but got ${args.size}.")
        }
        return callee.call(this, args)
    }

    private fun evaluate(expr: Expr): Any? {
        return expr.accept(this)
    }

    private fun isTruthy(obj: Any?): Boolean {
        if (obj == null) return false
        if (obj is Boolean) return obj
        return true
    }

    private fun isEqual(left: Any?, right: Any?): Boolean {
        return left == right
    }

    private fun checkNumberOperand(operator: Token, operand: Any?) {
        if (operand is Double) return
        throw RuntimeError(operator, "Operand must be a number.")
    }

    private fun checkNumberOperands(operator: Token, left: Any?, right: Any?) {
        if (left is Double && right is Double) return
        throw RuntimeError(operator, "Operands must be a numbers.")
    }

    private fun stringify(value: Any?): String {
        if (value == null) return "nil"

        if (value is Double) {
            var text = value.toString()
            if (text.endsWith(".0")) {
                text = text.substring(0, text.length - 2)
            }
            return text
        }

        return value.toString()
    }

    fun executeBlock(
        statements: List<Statement?>,
        environment: Environment,
    ) {
        val previous = this.environment
        try {
            this.environment = environment

            for (statement in statements) {
                execute(statement)
            }
        } finally {
            this.environment = previous
        }
    }

    override fun visitExpressionStmt(stmt: Statement.Expression) {
        evaluate(stmt.expr)
    }

    override fun visitPrintStmt(stmt: Statement.Print) {
        val value = evaluate(stmt.expr)
        println(stringify(value))
    }

    override fun visitVarStmt(stmt: Statement.Var) {
        var value: Any? = null
        if (stmt.initializer != null) {
            value = evaluate(stmt.initializer)
        }

        environment.define(stmt.name.lexeme, value)
    }

    override fun visitBlockStmt(stmt: Statement.Block) {
        executeBlock(stmt.statements, Environment(environment))
    }

    override fun visitIfStmt(stmt: Statement.If) {
        if (isTruthy(evaluate(stmt.condition))) {
            execute(stmt.thenBranch)
        } else if (stmt.elseBranch != null) {
            execute(stmt.elseBranch)
        }
    }

    override fun visitWhileStmt(stmt: Statement.While) {
        while (isTruthy(evaluate(stmt.condition))) {
            execute(stmt.body)
        }
    }

    override fun visitFunctionStmt(stmt: Statement.Function) {
        val function = LoxFunction(stmt)
        environment.define(stmt.name.lexeme, function)
    }

    override fun visitReturnStmt(stmt: Statement.Return) {
        val value = if (stmt.value == null) null else evaluate(stmt.value)
        throw Return(value)
    }
}