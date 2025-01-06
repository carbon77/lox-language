package org.zakat.parser

import org.zakat.construct.Expr
import org.zakat.construct.Visitor

class AstPrinter : Visitor<String> {
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
}