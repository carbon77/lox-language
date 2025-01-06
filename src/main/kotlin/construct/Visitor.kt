package org.zakat.construct

interface Visitor<T> {
    fun visitBinaryExpression(expr: Expr.Binary): T
    fun visitGroupingExpression(expr: Expr.Grouping): T
    fun visitLiteralExpression(expr: Expr.Literal): T
    fun visitUnaryExpression(expr: Expr.Unary): T

    fun visitExpressionStmt(expr: Statement.Expression): T
    fun visitPrintStmt(expr: Statement.Print): T
}
