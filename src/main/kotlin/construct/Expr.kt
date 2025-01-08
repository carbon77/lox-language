package org.zakat.construct

import org.zakat.lexer.Token

interface Expr {
    fun <T> accept(visitor: ExpressionVisitor<T>): T

    data class Assign(val name: Token, val value: Expr) : Expr {
        override fun <T> accept(visitor: ExpressionVisitor<T>): T {
            return visitor.visitAssignExpression(this)
        }
    }

    data class Binary(val left: Expr, val operator: Token, val right: Expr) : Expr {
        override fun <T> accept(visitor: ExpressionVisitor<T>): T {
            return visitor.visitBinaryExpression(this)
        }
    }

    data class Logical(val left: Expr, val operator: Token, val right: Expr) : Expr {
        override fun <T> accept(visitor: ExpressionVisitor<T>): T {
            return visitor.visitLogicalExpression(this)
        }
    }

    data class Grouping(val expr: Expr) : Expr {
        override fun <T> accept(visitor: ExpressionVisitor<T>): T {
            return visitor.visitGroupingExpression(this)
        }
    }

    data class Literal(val value: Any?) : Expr {
        override fun <T> accept(visitor: ExpressionVisitor<T>): T {
            return visitor.visitLiteralExpression(this)
        }
    }

    data class Unary(val operator: Token, val right: Expr) : Expr {
        override fun <T> accept(visitor: ExpressionVisitor<T>): T {
            return visitor.visitUnaryExpression(this)
        }
    }

    data class Variable(val name: Token) : Expr {
        override fun <T> accept(visitor: ExpressionVisitor<T>): T {
            return visitor.visitVariableExpression(this)
        }
    }

    data class Call(val callee: Expr, val paren: Token, val arguments: List<Expr>): Expr {
        override fun <T> accept(visitor: ExpressionVisitor<T>): T {
            return visitor.visitCallExpression(this)
        }
    }
}