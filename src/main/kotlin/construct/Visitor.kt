package org.zakat.construct

interface Visitor<T> {
    fun visitBinaryExpression(expr: Expression.Binary): T
    fun visitGroupingExpression(expr: Expression.Grouping): T
    fun visitLiteralExpression(expr: Expression.Literal): T
    fun visitUnaryExpression(expr: Expression.Unary): T
}
