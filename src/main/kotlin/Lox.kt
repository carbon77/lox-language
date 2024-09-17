package org.zakat

import org.zakat.lexer.Lexer
import org.zakat.lexer.Token
import java.io.BufferedReader
import java.io.InputStreamReader
import java.nio.charset.Charset
import java.nio.file.Files
import java.nio.file.Paths
import java.util.*
import kotlin.system.exitProcess

class Lox {
    var hadError = false

    companion object {
        fun error(line: Int, message: String) {
            report(line, "", message)
        }

        private fun report(line: Int, where: String, message: String) {
            System.err.println("[line $line] Error $where: $message")
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
        }
    }

    private fun runCode(source: String) {
        val scanner = Lexer(source)
        val tokens: List<Token> = scanner.scanTokens()

        for (token in tokens) {
            println(token)
        }
    }

}

