package org.zakat.parser

import org.zakat.Lox
import org.zakat.construct.Expr
import org.zakat.construct.Statement
import org.zakat.lexer.Token
import org.zakat.lexer.TokenType
import kotlin.math.exp

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
            if (match(TokenType.CLASS)) return classDeclaration()
            if (match(TokenType.FUN)) return function("function")
            if (match(TokenType.VAR)) return varDeclaration()
            return statement()
        } catch (e: ParseError) {
            synchronize()
            return null
        }
    }

    private fun classDeclaration(): Statement {
        val name = consume(TokenType.IDENTIFIER, "Expect class name.")

        var superClass: Expr.Variable? = null
        if (match(TokenType.LESS)) {
            consume(TokenType.IDENTIFIER, "Expect superclass name.")
            superClass = Expr.Variable(previous())
        }

        consume(TokenType.LEFT_BRACE, "Expect '{' before class body.")

        val methods = mutableListOf<Statement.Function>()
        while (!check(TokenType.RIGHT_BRACE) && hasNext()) {
            methods.add(function("method"))
        }
        consume(TokenType.RIGHT_BRACE, "Expect '}' after class body.")
        return Statement.Class(name, superClass, methods)
    }

    // We use the same grammar rule for functions and class methods,
    // because we need its own error messages
    // kind parameter indicates whether it's function of class method
    private fun function(kind: String): Statement.Function {
        val name = consume(
            TokenType.IDENTIFIER,
            "Expect $kind name."
        )

        // Parsing parameters list
        consume(TokenType.LEFT_PAREN, "Expect '(' after $kind name.")
        val params = mutableListOf<Token>()
        if (!check(TokenType.RIGHT_PAREN)) {
            do {
                if (params.size >= 255) {
                    error(peek(), "Can't have more than 255 parameters.")
                }
                params.add(
                    consume(TokenType.IDENTIFIER, "Expect parameter name.")
                )
            } while (match(TokenType.COMMA))
        }
        consume(TokenType.RIGHT_PAREN, "Expect ')' after parameters.")

        // Parsing function body
        consume(TokenType.LEFT_BRACE, "Expect '{' before $kind body.")
        val body = block()
        return Statement.Function(name, params, body)
    }

    private fun statement(): Statement {
        return when {
            match(TokenType.PRINT) -> printStatement()
            match(TokenType.LEFT_BRACE) -> Statement.Block(block())
            match(TokenType.IF) -> ifStatement()
            match(TokenType.WHILE) -> whileStatement()
            match(TokenType.FOR) -> forStatement()
            match(TokenType.RETURN) -> returnStatement()
            else -> expressionStatement()
        }
    }

    private fun returnStatement(): Statement {
        val keyword = previous()
        val value: Expr? = if (check(TokenType.SEMICOLON)) null else expression()
        consume(TokenType.SEMICOLON, "Expect ';' after return value.")
        return Statement.Return(keyword, value)
    }

    // We don't create class for 'for loop'.
    // We use 'desugaring' to parse 'for loop' to 'while loop'
    private fun forStatement(): Statement {
        consume(TokenType.LEFT_PAREN, "Expect '(' after 'for'.")
        val initializer: Statement? = when {
            match(TokenType.SEMICOLON) -> null
            match(TokenType.VAR) -> varDeclaration()
            else -> expressionStatement()
        }

        var condition = if (check(TokenType.SEMICOLON)) null else expression()
        consume(TokenType.SEMICOLON, "Expect ';' after loop condition.")

        val increment = if (check(TokenType.RIGHT_PAREN)) null else expression()
        consume(TokenType.RIGHT_PAREN, "Expect ')' after for clauses.")

        var body = statement()

        if (increment != null) {
            body = Statement.Block(
                listOf(
                    body,
                    Statement.Expression(increment),
                )
            )
        }

        if (condition == null) condition = Expr.Literal(true)
        body = Statement.While(condition, body)

        if (initializer != null) {
            body = Statement.Block(
                listOf(
                    initializer,
                    body,
                )
            )
        }

        return body
    }

    private fun whileStatement(): Statement {
        consume(TokenType.LEFT_PAREN, "Expect '(' after 'while'.")
        val condition = expression()
        consume(TokenType.RIGHT_PAREN, "Expect ')' after while condition.")
        val body = statement()

        return Statement.While(condition, body)
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

    // The function assumes that LEFT_BRACE token has already been consumed
    // to form more precise error message
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
        val expr = or()

        if (match(TokenType.EQUAL)) {
            val equals = previous()
            val value = assignment()

            if (expr is Expr.Variable) {
                return Expr.Assign(expr.name, value)
            } else if (expr is Expr.Get) {
                return Expr.Set(expr.obj, expr.name, value)
            }

            error(equals, "Invalid assignment target.")
        }

        return expr
    }

    private fun or(): Expr {
        var expr = and()

        while (match(TokenType.OR)) {
            val operator = previous()
            val right = and()
            expr = Expr.Logical(expr, operator, right)
        }

        return expr
    }

    private fun and(): Expr {
        var expr = equality()

        while (match(TokenType.AND)) {
            val operator = previous()
            val right = equality()
            expr = Expr.Logical(expr, operator, right)
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
        return call()
    }

    private fun call(): Expr {
        var expr = primary()

        while (true) {
            if (match(TokenType.LEFT_PAREN)) {
                expr = finishCall(expr)
            } else if (match(TokenType.DOT)) {
                val name = consume(TokenType.IDENTIFIER,
                    "Expect property name after '.'.")
                expr = Expr.Get(expr, name)
            } else {
                break
            }
        }

        return expr
    }

    private fun finishCall(callee: Expr): Expr {
        val args = mutableListOf<Expr>()
        if (!check(TokenType.RIGHT_PAREN)) {
            do {
                if (args.size >= 255) {
                    error(peek(), "Can't have more than 255 arguments.")
                }
                args.add(expression())
            } while (match(TokenType.COMMA))
        }
        val paren = consume(
            TokenType.RIGHT_PAREN,
            "Expect ')' after arguments."
        )
        return Expr.Call(callee, paren, args)
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
            match(TokenType.THIS) -> Expr.This(previous())
            match(TokenType.IDENTIFIER) -> Expr.Variable(previous())
            match(TokenType.SUPER) -> {
                val keyword = previous()
                consume(TokenType.DOT, "Expect '.' after 'super'.")
                val method = consume(TokenType.IDENTIFIER,
                    "Expect superclass method name.")
                Expr.Super(keyword, method)
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