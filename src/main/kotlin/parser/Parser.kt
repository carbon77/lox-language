package org.zakat.parser

import org.zakat.Lox
import org.zakat.construct.Expr
import org.zakat.construct.Statement
import org.zakat.lexer.Token
import org.zakat.lexer.TokenType

class Parser(
    private val tokens: List<Token>,
) {
    private var current = 0

    fun parse(): List<Statement?> {
        val statements = mutableListOf<Statement?>()
        while (hasNext()) {
            statements.add(declaration())
        }

        return statements
    }

    private fun declaration(): Statement? {
        try {
            if (match(TokenType.VAR)) return varDeclaration()
            return statement()
        } catch (e: ParseError) {
            synchronize()
            return null
        }
    }

    private fun statement(): Statement {
        return when {
            match(TokenType.PRINT) -> printStatement()
            match(TokenType.LEFT_BRACE) -> Statement.Block(block())
            match(TokenType.IF) -> ifStatement()
            else -> expressionStatement()
        }
    }

    private fun ifStatement(): Statement {
        consume(TokenType.LEFT_PAREN, "Expect '(' after 'if'.")
        val condition = expression()
        consume(TokenType.RIGHT_PAREN, "Expect ')' after if condition.")

        val thenBranch = statement()
        var elseBranch: Statement? = null
        if (match(TokenType.ELSE)) {
            elseBranch = statement()
        }

        return Statement.If(condition, thenBranch, elseBranch)
    }

    private fun varDeclaration(): Statement {
        val name = consume(TokenType.IDENTIFIER, "Expect variable name")

        var initializer: Expr? = null
        if (match(TokenType.EQUAL)) {
            initializer = expression()
        }

        consume(TokenType.SEMICOLON, "Expect ';' after variable declaration.")
        return Statement.Var(name, initializer)
    }

    private fun block(): List<Statement?> {
        val statements = mutableListOf<Statement?>()

        while (!check(TokenType.RIGHT_BRACE) && hasNext()) {
            statements.add(declaration())
        }

        consume(TokenType.RIGHT_BRACE, "Expect '}' after block.")
        return statements
    }

    private fun printStatement(): Statement {
        val value = expression()
        consume(TokenType.SEMICOLON, "Expect ';' after value.")
        return Statement.Print(value)
    }

    private fun expressionStatement(): Statement {
        val expr = expression()
        consume(TokenType.SEMICOLON, "Expect ';' after expression.")
        return Statement.Expression(expr)
    }

    private fun expression(): Expr {
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
        return assignment()
    }

    private fun assignment(): Expr {
        val expr = equality()

        if (match(TokenType.EQUAL)) {
            val equals = previous()
            val value = assignment()

            if (expr is Expr.Variable) {
                return Expr.Assign(expr.name, value)
            }

            error(equals, "Invalid assignment target.")
        }

        return expr
    }

    // Grammar rule for !=, ==
    private fun equality(): Expr {
        var expression = comparison()

        while (match(TokenType.BANG_EQUAL, TokenType.EQUAL_EQUAL)) {
            val operator = previous()
            val right = comparison()
            expression = Expr.Binary(expression, operator, right)
        }

        return expression
    }

    // Grammar rule for >, >=, <, <=
    private fun comparison(): Expr {
        var expression = term()

        while (match(
                TokenType.GREATER, TokenType.GREATER_EQUAL,
                TokenType.LESS, TokenType.LESS_EQUAL,
            )
        ) {
            val operator = previous()
            val right = term()
            expression = Expr.Binary(expression, operator, right)
        }

        return expression
    }

    // Grammar rule for -, +
    private fun term(): Expr {
        var expression = factor()

        while (match(TokenType.MINUS, TokenType.PLUS)) {
            val operator = previous()
            val right = factor()
            expression = Expr.Binary(expression, operator, right)
        }

        return expression
    }

    // Grammar rule for /, *
    private fun factor(): Expr {
        var expression = unary()

        while (match(TokenType.SLASH, TokenType.STAR)) {
            val operator = previous()
            val right = unary()
            expression = Expr.Binary(expression, operator, right)
        }

        return expression
    }

    private fun unary(): Expr {
        if (match(TokenType.BANG, TokenType.MINUS)) {
            val operator = previous()
            val right = unary()
            return Expr.Unary(operator, right)
        }
        return primary()
    }

    private fun primary(): Expr {
        return when {
            match(TokenType.FALSE) -> Expr.Literal(false)
            match(TokenType.TRUE) -> Expr.Literal(true)
            match(TokenType.NIL) -> Expr.Literal(null)
            match(TokenType.NUMBER, TokenType.STRING) -> Expr.Literal(previous().literal)
            match(TokenType.LEFT_PAREN) -> {
                val expression = expression()
                consume(TokenType.RIGHT_PAREN, "Expect ')' after expression")
                expression
            }

            match(TokenType.IDENTIFIER) -> Expr.Variable(previous())

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