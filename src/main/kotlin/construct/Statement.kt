package org.zakat.construct

import org.zakat.lexer.Token

interface Statement {
    fun accept(visitor: StatementVisitor)

    data class Block(val statements: List<Statement?>) : Statement {
        override fun accept(visitor: StatementVisitor) {
            return visitor.visitBlockStmt(this)
        }
    }

    data class Expression(val expr: Expr) : Statement {
        override fun accept(visitor: StatementVisitor) {
            return visitor.visitExpressionStmt(this)
        }
    }

    data class Print(val expr: Expr) : Statement {
        override fun accept(visitor: StatementVisitor) {
            return visitor.visitPrintStmt(this)
        }
    }

    data class Var(val name: Token, val initializer: Expr?) : Statement {
        override fun accept(visitor: StatementVisitor) {
            return visitor.visitVarStmt(this)
        }
    }
}
