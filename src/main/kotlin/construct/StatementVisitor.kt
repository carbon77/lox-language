package org.zakat.construct

interface StatementVisitor<T> {
    fun visitExpressionStmt(stmt: Statement.Expression): T
    fun visitPrintStmt(stmt: Statement.Print): T
}