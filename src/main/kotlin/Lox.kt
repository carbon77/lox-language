package org.zakat

import org.zakat.lexer.Lexer
import org.zakat.lexer.Token
import org.zakat.lexer.TokenType
import org.zakat.parser.AstPrinter
import org.zakat.parser.Parser
import java.io.BufferedReader
import java.io.InputStreamReader
import java.nio.charset.Charset
import java.nio.file.Files
import java.nio.file.Paths
import kotlin.system.exitProcess

class Lox {

    companion object {
        var hadError = false

        fun error(line: Int, message: String) {
            report(line, "", message)
        }

        fun error(token: Token, message: String) {
            if (token.type == TokenType.EOF) {
                report(token.line, "at end", message)
            } else {
                report(token.line, "at '${token.lexeme}'", message)
            }
        }

        private fun report(line: Int, where: String, message: String) {
            System.err.println("[line $line] Error $where: $message")
            hadError = true
        }
    }

    fun runFile(path: String) {
        val bytes = Files.readAllBytes(Paths.get(path))
        runCode(String(bytes, Charset.defaultCharset()))

        if (hadError) exitProcess(65)
    }

    fun runPrompt() {
        val input = InputStreamReader(System.`in`)
        val reader = BufferedReader(input)

        while (true) {
            print("> ")
            val line = reader.readLine() ?: break
            runCode(line)
            hadError = false
        }
    }

    private fun runCode(source: String) {
        val scanner = Lexer(source)
        val tokens: List<Token> = scanner.scanTokens()

        val parser = Parser(tokens)
        val expression = parser.parse()

        if (expression == null || hadError) return

        println(AstPrinter().print(expression))
    }
}

