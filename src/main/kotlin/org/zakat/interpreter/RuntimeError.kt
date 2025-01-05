package org.zakat.org.zakat.interpreter

import org.zakat.lexer.Token

data class RuntimeError(
    val token: Token,
    val s: String
) : RuntimeException(s)
