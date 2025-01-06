package parser

import org.junit.jupiter.api.Test
import org.zakat.construct.Expression
import org.zakat.lexer.Token
import org.zakat.lexer.TokenType
import org.zakat.parser.AstPrinter
import kotlin.test.assertEquals

class AstPrinterTest {

    @Test
    fun print() {
        val expression = Expression.Binary(
            Expression.Unary(
                Token(TokenType.MINUS, "-", null, 1),
                Expression.Literal(123),
            ),
            Token(TokenType.STAR, "*", null, 1),
            Expression.Grouping(
                Expression.Binary(
                    Expression.Literal(23),
                    Token(TokenType.PLUS, "+", null, 1),
                    Expression.Literal(43.01),
                )
            ),
        )
        val printer = AstPrinter()
        val result = printer.print(expression)

        assertEquals(result, "(* (- 123) (group (+ 23 43.01)))")
    }
}