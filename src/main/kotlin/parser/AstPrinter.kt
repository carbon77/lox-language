package org.zakat.parser

class AstPrinter : Expression.Visitor<String> {
    fun print(expr: Expression): String {
        return expr.accept(this)
    }

    private fun parenthesize(name: String, vararg expressions: Expression): String {
        val builder = StringBuilder()

        builder.append("(").append(name)
        for (expression in expressions) {
            builder.append(" ")
            builder.append(expression.accept(this))
        }
        builder.append(")")
        return builder.toString()
    }

    override fun visitBinaryExpression(expr: Expression.Binary): String {
        return parenthesize(expr.operator.lexeme, expr.left, expr.right)
    }

    override fun visitGroupingExpression(expr: Expression.Grouping): String {
        return parenthesize("group", expr.expr)
    }

    override fun visitLiteralExpression(expr: Expression.Literal): String {
        return if (expr.value == null) "nil" else expr.value.toString()
    }

    override fun visitUnaryExpression(expr: Expression.Unary): String {
        return parenthesize(expr.operator.lexeme, expr.right)
    }
}