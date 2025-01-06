package org.zakat.parser

import org.zakat.Lox
import org.zakat.construct.Expression
import org.zakat.lexer.Token
import org.zakat.lexer.TokenType

class Parser(
    private val tokens: List<Token>,
) {
    private var current = 0

    fun parse(): Expression? {
        return try {
            expression()
        } catch (e: ParseError) {
            null
        }
    }

    private fun expression(): Expression {
        if (check(
                TokenType.EQUAL,
                TokenType.EQUAL_EQUAL,
                TokenType.BANG_EQUAL,
                TokenType.GREATER,
                TokenType.GREATER_EQUAL,
                TokenType.LESS,
                TokenType.LESS_EQUAL,
                TokenType.PLUS,
                TokenType.STAR,
                TokenType.SLASH
            )
        ) {
            throw error(peek(), "Unexpected binary operator")
        }
        return equality()
    }

    // Grammar rule for !=, ==
    private fun equality(): Expression {
        var expression = comparison()

        while (match(TokenType.BANG_EQUAL, TokenType.EQUAL_EQUAL)) {
            val operator = previous()
            val right = comparison()
            expression = Expression.Binary(expression, operator, right)
        }

        return expression
    }

    // Grammar rule for >, >=, <, <=
    private fun comparison(): Expression {
        var expression = term()

        while (match(
                TokenType.GREATER, TokenType.GREATER_EQUAL,
                TokenType.LESS, TokenType.LESS_EQUAL,
            )
        ) {
            val operator = previous()
            val right = term()
            expression = Expression.Binary(expression, operator, right)
        }

        return expression
    }

    // Grammar rule for -, +
    private fun term(): Expression {
        var expression = factor()

        while (match(TokenType.MINUS, TokenType.PLUS)) {
            val operator = previous()
            val right = factor()
            expression = Expression.Binary(expression, operator, right)
        }

        return expression
    }

    // Grammar rule for /, *
    private fun factor(): Expression {
        var expression = unary()

        while (match(TokenType.SLASH, TokenType.STAR)) {
            val operator = previous()
            val right = unary()
            expression = Expression.Binary(expression, operator, right)
        }

        return expression
    }

    private fun unary(): Expression {
        if (match(TokenType.BANG, TokenType.MINUS)) {
            val operator = previous()
            val right = unary()
            return Expression.Unary(operator, right)
        }
        return primary()
    }

    private fun primary(): Expression {
        return when {
            match(TokenType.FALSE) -> Expression.Literal(false)
            match(TokenType.TRUE) -> Expression.Literal(true)
            match(TokenType.NIL) -> Expression.Literal(null)
            match(TokenType.NUMBER, TokenType.STRING) -> Expression.Literal(previous().literal)
            match(TokenType.LEFT_PAREN) -> {
                val expression = expression()
                consume(TokenType.RIGHT_PAREN, "Expect ')' after expression")
                expression
            }

            else -> throw error(peek(), "Expect expression")
        }
    }

    // Like match but throws an exception
    private fun consume(type: TokenType, message: String): Token {
        if (check(type)) return advance()
        throw error(peek(), message)
    }

    private fun error(token: Token, message: String): ParseError {
        Lox.error(token, message)
        return ParseError()
    }

    private fun match(vararg types: TokenType): Boolean {
        for (type in types) {
            if (check(type)) {
                advance()
                return true
            }
        }
        return false
    }

    private fun check(vararg types: TokenType): Boolean {
        if (!hasNext()) return false

        for (type in types) {
            if (peek().type == type) {
                return true
            }
        }
        return false
    }

    private fun advance(): Token {
        if (hasNext()) current++
        return previous()
    }

    private fun hasNext() = peek().type != TokenType.EOF
    private fun peek() = tokens[current]
    private fun previous() = tokens[current - 1]

    private fun synchronize() {
        advance()

        while (hasNext()) {
            if (previous().type == TokenType.SEMICOLON) return

            when (peek().type) {
                TokenType.CLASS,
                TokenType.FUN,
                TokenType.VAR,
                TokenType.FOR,
                TokenType.IF,
                TokenType.WHILE,
                TokenType.PRINT,
                TokenType.RETURN -> return

                else -> advance()
            }
        }
    }
}