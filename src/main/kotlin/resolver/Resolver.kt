package org.zakat.resolver

import org.zakat.Lox
import org.zakat.construct.Expr
import org.zakat.construct.ExpressionVisitor
import org.zakat.construct.Statement
import org.zakat.construct.StatementVisitor
import org.zakat.interpreter.Interpreter
import org.zakat.lexer.Token
import java.util.Stack

class Resolver(
    private val interpreter: Interpreter,
) : ExpressionVisitor<Unit>, StatementVisitor {
    private val scopes = Stack<MutableMap<String, Boolean>>()
    private var currentFunction = FunctionType.NONE

    override fun visitBinaryExpression(expr: Expr.Binary) {
        resolve(expr.left)
        resolve(expr.right)
    }

    override fun visitGroupingExpression(expr: Expr.Grouping) {
        resolve(expr.expr)
    }

    override fun visitLiteralExpression(expr: Expr.Literal) {
    }

    override fun visitUnaryExpression(expr: Expr.Unary) {
        resolve(expr.right)
    }

    override fun visitVariableExpression(expr: Expr.Variable) {
        if (!scopes.isEmpty() && scopes.peek()[expr.name.lexeme] == false) {
            Lox.error(expr.name,
                "Can't read local variable in its own initializer.")
        }
        resolveLocal(expr, expr.name)
    }

    override fun visitAssignExpression(expr: Expr.Assign) {
        resolve(expr.value)
        resolveLocal(expr, expr.name)
    }

    override fun visitLogicalExpression(expr: Expr.Logical) {
        resolve(expr.left)
        resolve(expr.right)
    }

    override fun visitCallExpression(expr: Expr.Call) {
        resolve(expr.callee)

        for (arg in expr.arguments) {
            resolve(arg)
        }
    }

    override fun visitExpressionStmt(stmt: Statement.Expression) {
        resolve(stmt.expr)
    }

    override fun visitPrintStmt(stmt: Statement.Print) {
        resolve(stmt.expr)
    }

    override fun visitVarStmt(stmt: Statement.Var) {
        declare(stmt.name)
        if (stmt.initializer != null) {
            resolve(stmt.initializer)
        }
        define(stmt.name)
    }

    override fun visitBlockStmt(stmt: Statement.Block) {
        beginScope()
        resolve(stmt.statements)
        endScope()
    }

    override fun visitIfStmt(stmt: Statement.If) {
        resolve(stmt.condition)
        resolve(stmt.thenBranch)
        if (stmt.elseBranch != null) resolve(stmt.elseBranch)
    }

    override fun visitWhileStmt(stmt: Statement.While) {
        resolve(stmt.condition)
        resolve(stmt.body)
    }

    override fun visitFunctionStmt(stmt: Statement.Function) {
        declare(stmt.name)
        define(stmt.name)

        resolveFunction(stmt, FunctionType.FUNCTION)
    }

    override fun visitReturnStmt(stmt: Statement.Return) {
        if (currentFunction == FunctionType.NONE) {
            Lox.error(stmt.keyword, "Can't return from top-level code.")
        }

        if (stmt.value != null) resolve(stmt.value)
    }

    private fun beginScope() {
        scopes.push(mutableMapOf())
    }

    private fun endScope() {
        scopes.pop()
    }

    fun resolve(statements: List<Statement?>) {
        for (statement in statements) {
            resolve(statement)
        }
    }

    private fun resolve(statement: Statement?) {
        statement?.accept(this)
    }

    private fun resolve(expr: Expr) {
        expr.accept(this)
    }

    private fun declare(name: Token) {
        if (scopes.isEmpty()) return

        val scope = scopes.peek()
        if (name.lexeme in scope) {
            Lox.error(name,
                "Already variable with this name in this scope.")
        }
        scope[name.lexeme] = false
    }

    private fun define(name: Token) {
        if (scopes.isEmpty()) return
        scopes.peek()[name.lexeme] = true
    }

    private fun resolveLocal(expr: Expr, name: Token) {
        for (i in scopes.size - 1 downTo 0) {
            if (scopes[i].containsKey(name.lexeme)) {
                interpreter.resolve(expr, scopes.size - i - 1)
                return
            }
        }
    }

    private fun resolveFunction(function: Statement.Function, type: FunctionType) {
        val enclosingFunction = currentFunction
        currentFunction = type
        beginScope()
        for (param in function.params) {
            declare(param)
            define(param)
        }
        resolve(function.body)
        endScope()
        currentFunction = enclosingFunction
    }
}