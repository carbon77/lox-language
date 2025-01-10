package lexer

import org.junit.jupiter.api.Assertions.assertEquals
import org.junit.jupiter.api.Test
import org.zakat.lexer.Lexer
import org.zakat.lexer.Token
import org.zakat.lexer.TokenType

class LexerTest {

    @Test
    fun scanTokens() {
        val source = "123.23\n 32 + - ! != == if fun x \"Hello world\" /"
        val lexer = Lexer(source)
        val tokens = lexer.scanTokens()

        assertEquals(
            tokens, listOf(
                Token(TokenType.NUMBER, "123.23", 123.23, 1),
                Token(TokenType.NUMBER, "32", 32.0, 2),
                Token(TokenType.PLUS, "+", null, 2),
                Token(TokenType.MINUS, "-", null, 2),
                Token(TokenType.BANG, "!", null, 2),
                Token(TokenType.BANG_EQUAL, "!=", null, 2),
                Token(TokenType.EQUAL_EQUAL, "==", null, 2),
                Token(TokenType.IF, "if", null, 2),
                Token(TokenType.FUN, "fun", null, 2),
                Token(TokenType.IDENTIFIER, "x", null, 2),
                Token(TokenType.STRING, "\"Hello world\"", "Hello world", 2),
                Token(TokenType.SLASH, "/", null, 2),
                Token(TokenType.EOF, "", null, 2)
            )
        )
    }
}