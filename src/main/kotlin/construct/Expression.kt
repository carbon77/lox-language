package org.zakat.construct

import org.zakat.lexer.Token

interface Expression {
    fun <T> accept(visitor: Visitor<T>): T

    data class Binary(val left: Expression, val operator: Token, val right: Expression) : Expression {
        override fun <T> accept(visitor: Visitor<T>): T {
            return visitor.visitBinaryExpression(this)
        }
    }

    data class Grouping(val expr: Expression) : Expression {
        override fun <T> accept(visitor: Visitor<T>): T {
            return visitor.visitGroupingExpression(this)
        }
    }

    data class Literal(val value: Any?) : Expression {
        override fun <T> accept(visitor: Visitor<T>): T {
            return visitor.visitLiteralExpression(this)
        }
    }

    data class Unary(val operator: Token, val right: Expression) : Expression {
        override fun <T> accept(visitor: Visitor<T>): T {
            return visitor.visitUnaryExpression(this)
        }
    }
}