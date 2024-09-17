package org.zakat.lexer

import org.zakat.Lox

class Lexer(
    private val source: String
) {
    private val tokens: MutableList<Token> = mutableListOf()
    private var start = 0
    private var current = 0
    private var line = 1

    companion object {
        private val keywords: Map<String, TokenType> = mapOf(
            "and" to TokenType.AND,
            "class" to TokenType.CLASS,
            "else" to TokenType.ELSE,
            "false" to TokenType.FALSE,
            "for" to TokenType.FOR,
            "fun" to TokenType.FUN,
            "if" to TokenType.IF,
            "nil" to TokenType.NIL,
            "or" to TokenType.OR,
            "print" to TokenType.PRINT,
            "return" to TokenType.RETURN,
            "super" to TokenType.SUPER,
            "this" to TokenType.THIS,
            "true" to TokenType.TRUE,
            "var" to TokenType.VAR,
            "while" to TokenType.WHILE,
        )
    }

    fun scanTokens(): List<Token> {
        while (hasNext()) {
            start = current
            scanToken()
        }

        tokens.add(Token(TokenType.EOF, "", null, line))
        return tokens
    }

    private fun scanToken() {
        val c = advance()
        when (c) {
            '(' -> addToken(TokenType.LEFT_PAREN)
            ')' -> addToken(TokenType.RIGHT_PAREN)
            '{' -> addToken(TokenType.LEFT_BRACE)
            '}' -> addToken(TokenType.RIGHT_BRACE)
            ',' -> addToken(TokenType.COMMA)
            '.' -> addToken(TokenType.DOT)
            '-' -> addToken(TokenType.MINUS)
            '+' -> addToken(TokenType.PLUS)
            ';' -> addToken(TokenType.SEMICOLON)
            '*' -> addToken(TokenType.STAR)
            '!' -> addToken(if (match('=')) TokenType.BANG_EQUAL else TokenType.BANG)
            '=' -> addToken(if (match('=')) TokenType.EQUAL_EQUAL else TokenType.EQUAL)
            '<' -> addToken(if (match('=')) TokenType.LESS_EQUAL else TokenType.LESS)
            '>' -> addToken(if (match('=')) TokenType.GREATER_EQUAL else TokenType.GREATER)
            '/' -> {
                if (match('/')) {
                    while (peek() != '\n' && hasNext()) advance()
                } else {
                    addToken(TokenType.SLASH)
                }
            }

            '"' -> scanString()
            ' ', '\r', '\t' -> {}
            '\n' -> line++
            else -> {
                if (isDigit(c)) {
                    scanNumber()
                } else if (isLetter(c)) {
                    scanIdentifier()
                } else {
                    Lox.error(line, "Unexpected character")
                }
            }
        }
    }

    private fun scanIdentifier() {
        while (isLetterIsDigit(peek())) advance()

        val text = source.substring(start, current)
        var type = keywords[text]
        if (type == null) type = TokenType.IDENTIFIER
        addToken(type)
    }

    private fun scanNumber() {
        while (isDigit(peek())) advance()

        if (peek() == '.' && isDigit(peekNext())) {
            advance()

            while (isDigit(peek())) advance()
        }

        addToken(
            TokenType.NUMBER,
            source.substring(start, current).toDouble()
        )
    }

    private fun peekNext(): Char {
        if (current + 1 >= source.length) return 0.toChar()
        return source[current + 1]
    }

    private fun isLetter(c: Char): Boolean = (c in 'a'..'z') ||
            (c in 'A'..'Z') ||
            c == '_'

    private fun isDigit(c: Char): Boolean = c in '0'..'9'
    private fun isLetterIsDigit(c: Char): Boolean = isLetter(c) || isDigit(c)

    private fun scanString() {
        while (peek() != '"' && hasNext()) {
            if (peek() == '\n') line++
            advance()
        }

        if (!hasNext()) {
            Lox.error(line, "Unterminated string.")
            return
        }

        advance()
        val value = source.substring(start + 1, current - 1)
        addToken(TokenType.STRING, value)
    }

    private fun peek(): Char {
        if (!hasNext()) return 0.toChar()
        return source[current]
    }

    private fun match(expected: Char): Boolean {
        if (!hasNext()) return false
        if (source[current] != expected) return false

        current++
        return true
    }

    private fun advance(): Char {
        current++
        return source[current - 1]
    }

    private fun addToken(type: TokenType) {
        addToken(type, null)
    }

    private fun addToken(type: TokenType, literal: Any?) {
        val text = source.substring(start, current)
        tokens.add(Token(type, text, literal, line))
    }

    fun hasNext(): Boolean = current < source.length
}