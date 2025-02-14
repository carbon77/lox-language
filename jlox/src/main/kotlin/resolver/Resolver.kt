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
    private var currentClass = ClassType.NONE

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

    override fun visitGetExpression(expr: Expr.Get) {
        resolve(expr.obj)
    }

    override fun visitSetExpression(expr: Expr.Set) {
        resolve(expr.value)
        resolve(expr.obj)
    }

    override fun visitThisExpression(expr: Expr.This) {
        if (currentClass == ClassType.NONE) {
            Lox.error(expr.keyword,
                "Can't use 'this' outside of a class")
            return
        }
        resolveLocal(expr, expr.keyword)
    }

    override fun visitSuperExpression(expr: Expr.Super) {
        if (currentClass == ClassType.NONE) {
            Lox.error(expr.keyword,
                "Can't use 'super' outside of a class")
        } else if (currentClass != ClassType.SUBCLASS) {
            Lox.error(expr.keyword,
                "Can't use 'super' in a class with no superclass.")
        }
        resolveLocal(expr, expr.keyword)
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

        if (stmt.value != null) {
            if (currentFunction == FunctionType.INITIALIZER) {
                Lox.error(stmt.keyword, "Can't return from an initializer.")
            }
            resolve(stmt.value)
        }
    }

    override fun visitClassStmt(stmt: Statement.Class) {
        val enclosingClass = currentClass
        currentClass = ClassType.CLASS
        declare(stmt.name)
        define(stmt.name)

        if (stmt.superClass != null && stmt.name.lexeme == stmt.superClass.name.lexeme) {
            Lox.error(stmt.superClass.name,
                "A class can't inherit from itself.")
        }

        if (stmt.superClass != null) {
            currentClass = ClassType.SUBCLASS
            resolve(stmt.superClass)
            beginScope()
            scopes.peek()["super"] = true
        }

        beginScope()
        scopes.peek()["this"] = true

        for (method in stmt.methods) {
            var declaration = FunctionType.METHOD
            if (method.name.lexeme == "init") {
                declaration = FunctionType.INITIALIZER
            }
            resolveFunction(method, declaration)
        }

        endScope()

        if (stmt.superClass != null) endScope()

        currentClass = enclosingClass
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