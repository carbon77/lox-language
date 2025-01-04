package org.zakat.parser

import org.zakat.lexer.Token

interface Expression {
    fun <T> accept(visitor: Visitor<T>): T

    interface Visitor<T> {
        fun visitBinaryExpression(expr: Binary): T
        fun visitGroupingExpression(expr: Grouping): T
        fun visitLiteralExpression(expr: Literal): T
        fun visitUnaryExpression(expr: Unary): T
    }

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