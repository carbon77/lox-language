package org.zakat.construct

import org.zakat.lexer.Token

interface Expr {
    fun <T> accept(visitor: ExpressionVisitor<T>): T

    data class Binary(val left: Expr, val operator: Token, val right: Expr) : Expr {
        override fun <T> accept(visitor: ExpressionVisitor<T>): T {
            return visitor.visitBinaryExpression(this)
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
}