package org.zakat.interpreter

import org.zakat.Lox
import org.zakat.construct.Expr
import org.zakat.construct.ExpressionVisitor
import org.zakat.construct.Statement
import org.zakat.construct.StatementVisitor
import org.zakat.lexer.Token
import org.zakat.lexer.TokenType

class Interpreter : ExpressionVisitor<Any?>, StatementVisitor<Any?> {

    fun interpret(statements: List<Statement>) {
        try {
            for (statement in statements) {
                execute(statement)
            }
        } catch (e: RuntimeError) {
            Lox.runtimeError(e)
        }
    }

    private fun execute(statement: Statement) {
        statement.accept(this)
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

    override fun visitExpressionStmt(stmt: Statement.Expression) {
        evaluate(stmt.expr)
    }

    override fun visitPrintStmt(stmt: Statement.Print) {
        val value = evaluate(stmt.expr)
        println(stringify(value))
    }
}