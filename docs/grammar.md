### Current Grammar

#### Expressions: (add tuples and arrays later!!)
```bnf
<literal> ::= <boolean> | <integer> | <float> | <identifier>
<access> ::= <literal> ("::" <literal>)
<call> ::= <identifier> "(" (<expr> ",")* ")"
<unary> ::= "(" <expr> ")" | "-"? <access> | <call>
<factor> ::= <unary> (("*" | "/") <unary>)*
<term> ::= <factor> (("+" | "-") <factor>)*
<equality> ::= <term> (("==" | "!=") <term>)*
<compare> ::= <equality> (("<" | ">") <equality>)*
<and> ::= <compare> ("&&" <compare>)*
<or> ::= <and> ("||" <and>)*
<assign> ::= <unary> ("=" <or>)?
<expr> ::= <assign>
```

#### Statements:
```bnf
<comment> ::= "#" ... "#"
<program> ::= <top-stmt>*
<top-stmt> ::= <use-native> | <import> | <function-decl>
<use-native> ::= "use" "func" <identifier> <arg-list> ":" <type-specifier> ";"
<import> = "import" <identifier> ";"
<function-decl> ::= "func" <identifier> <arg-list> ":" <type-specifier> <block>
<arg-list> ::= "(" (<arg> ",")* ")"
<arg> = <identifier> ":" <type-specifier>
<type-specifier> ::= "bool" | "int" | "float" | "string"
<block> ::= "{" <nestable-stmt>+ "}"
<nestable-stmt> ::= <variable-decl> | <expr-stmt> | <return> | <if> | <while>
<variable-decl> ::= ("let" | "const") <identifier> ":" <type-specifier> "=" <or> ";"
<expr-stmt> ::= <assign> ";"
<return> ::= "return" <or> ";"
<if> ::= "if" "(" <or> ")" <block> ("else" <block>)?
<while> ::= "while" "(" <or> ")" <block>
```
