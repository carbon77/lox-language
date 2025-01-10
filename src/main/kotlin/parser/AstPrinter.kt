package org.zakat.parser

import org.zakat.construct.Expr
import org.zakat.construct.ExpressionVisitor

class AstPrinter : ExpressionVisitor<String> {
    fun print(expr: Expr): String {
        return expr.accept(this)
    }

    private fun parenthesize(name: String, vararg exprs: Expr): String {
        val builder = StringBuilder()

        builder.append("(").append(name)
        for (expression in exprs) {
            builder.append(" ")
            builder.append(expression.accept(this))
        }
        builder.append(")")
        return builder.toString()
    }

    override fun visitBinaryExpression(expr: Expr.Binary): String {
        return parenthesize(expr.operator.lexeme, expr.left, expr.right)
    }

    override fun visitGroupingExpression(expr: Expr.Grouping): String {
        return parenthesize("group", expr.expr)
    }

    override fun visitLiteralExpression(expr: Expr.Literal): String {
        return if (expr.value == null) "nil" else expr.value.toString()
    }

    override fun visitUnaryExpression(expr: Expr.Unary): String {
        return parenthesize(expr.operator.lexeme, expr.right)
    }

    override fun visitVariableExpression(expr: Expr.Variable): String {
        return expr.name.lexeme
    }

    override fun visitAssignExpression(expr: Expr.Assign): String {
        return parenthesize("=", expr.value)
    }

    override fun visitLogicalExpression(expr: Expr.Logical): String {
        return parenthesize(expr.operator.lexeme, expr.left, expr.right)
    }

    override fun visitCallExpression(expr: Expr.Call): String {
        return "${expr.callee}(${expr.arguments.joinToString(", ") { it.accept(this) }})"
    }

    override fun visitGetExpression(expr: Expr.Get): String {
        TODO("Not yet implemented")
    }

    override fun visitSetExpression(expr: Expr.Set): String {
        TODO("Not yet implemented")
    }
}