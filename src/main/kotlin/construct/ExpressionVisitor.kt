package org.zakat.construct

interface ExpressionVisitor<T> {
    fun visitBinaryExpression(expr: Expr.Binary): T
    fun visitGroupingExpression(expr: Expr.Grouping): T
    fun visitLiteralExpression(expr: Expr.Literal): T
    fun visitUnaryExpression(expr: Expr.Unary): T
    fun visitVariableExpression(expr: Expr.Variable): T
    fun visitAssignExpression(expr: Expr.Assign): T
    fun visitLogicalExpression(expr: Expr.Logical): T
    fun visitCallExpression(expr: Expr.Call): T
    fun visitGetExpression(expr: Expr.Get): T
    fun visitSetExpression(expr: Expr.Set): T
}
