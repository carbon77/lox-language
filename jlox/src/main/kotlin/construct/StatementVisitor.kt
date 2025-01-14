package org.zakat.construct

interface StatementVisitor {
    fun visitExpressionStmt(stmt: Statement.Expression)
    fun visitPrintStmt(stmt: Statement.Print)
    fun visitVarStmt(stmt: Statement.Var)
    fun visitBlockStmt(stmt: Statement.Block)
    fun visitIfStmt(stmt: Statement.If)
    fun visitWhileStmt(stmt: Statement.While)
    fun visitFunctionStmt(stmt: Statement.Function)
    fun visitReturnStmt(stmt: Statement.Return)
    fun visitClassStmt(stmt: Statement.Class)
}