package org.zakat.construct

interface StatementVisitor<T> {
    fun visitExpressionStmt(expr: Statement.Expression): T
    fun visitPrintStmt(expr: Statement.Print): T
}