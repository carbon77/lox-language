package org.zakat.construct

interface ExpressionVisitor<T> {
    fun visitBinaryExpression(expr: Expr.Binary): T
    fun visitGroupingExpression(expr: Expr.Grouping): T
    fun visitLiteralExpression(expr: Expr.Literal): T
    fun visitUnaryExpression(expr: Expr.Unary): T
}
