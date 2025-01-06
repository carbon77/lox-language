package org.zakat.construct

interface StatementVisitor {
    fun visitExpressionStmt(stmt: Statement.Expression)
    fun visitPrintStmt(stmt: Statement.Print)
    fun visitVarStmt(stmt: Statement.Var)
    fun visitBlockStmt(stmt: Statement.Block)
    fun visitIfStmt(stmt: Statement.If)
}