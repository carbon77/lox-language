package org.zakat.lexer

data class Token(
    val type: TokenType,
    val lexeme: String,
    val literal: Any?,
    val line: Int
) {

    override fun toString(): String {
        return "$line: $type $lexeme $literal"
    }
}