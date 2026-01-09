```
program        ::= (classDecl | fnDecl | stmt)* EOF
classDecl      ::= "CLASS" IDENTIFIER ("INHERITS" IDENTIFIER)? "ATTRIBUTES" ":" attrList "METHODS" ":" fnDecl* "END" IDENTIFIER
fnDecl         ::= "FUNCTION" IDENTIFIER "(" params? ")" block "END" IDENTIFIER
attrList       ::= (IDENTIFIER ("=" expr)?)*
block          ::= stmt*
stmt           ::= "RETURN" expr
                 | "IF" expr "THEN" block ("ELSE" block)? "END" "IF"
                 | "WHILE" expr block "END" "WHILE"
                 | "PRINT" "(" expr ")"
                 | expr         // Expression statement (assignment, calls)
expr           ::= IDENTIFIER | LITERAL | binary | unary | call | get | set | arrayLit
```