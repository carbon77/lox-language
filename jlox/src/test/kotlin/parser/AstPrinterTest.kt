package parser

import org.junit.jupiter.api.Test
import org.zakat.construct.Expr
import org.zakat.lexer.Token
import org.zakat.lexer.TokenType
import org.zakat.parser.AstPrinter
import kotlin.test.assertEquals

class AstPrinterTest {

    @Test
    fun print() {
        val expr = Expr.Binary(
            Expr.Unary(
                Token(TokenType.MINUS, "-", null, 1),
                Expr.Literal(123),
            ),
            Token(TokenType.STAR, "*", null, 1),
            Expr.Grouping(
                Expr.Binary(
                    Expr.Literal(23),
                    Token(TokenType.PLUS, "+", null, 1),
                    Expr.Literal(43.01),
                )
            ),
        )
        val printer = AstPrinter()
        val result = printer.print(expr)

        assertEquals(result, "(* (- 123) (group (+ 23 43.01)))")
    }
}