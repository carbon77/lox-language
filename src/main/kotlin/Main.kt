package org.zakat

import kotlin.system.exitProcess

//TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or
// click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
fun main(args: Array<String>) {
    val lox = Lox()
    if (args.size > 1) {
        println("Usage: jlox [script]")
        exitProcess(64)
    } else if (args.size == 1) {
        lox.runFile(args[0])
    } else {
        lox.runPrompt()
    }
}
