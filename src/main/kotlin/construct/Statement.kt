package org.zakat.construct

interface Statement {
    fun <T> visit(visitor: Visitor<T>): T

    data class Expression(
        val expr: Expr,
    ) : Statement {
        override fun <T> visit(visitor: Visitor<T>): T {
            return visitor.visitExpressionStmt(this)
        }
    }

    data class Print(
        val expr: Expr,
    ) : Statement {
        override fun <T> visit(visitor: Visitor<T>): T {
            return visitor.visitPrintStmt(this)
        }
    }
}
