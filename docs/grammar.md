### Current Grammar

#### Expressions: (add tuples and arrays later!!)
```bnf
<literal> = <boolean> / <integer> / <identifier>
<access> = <literal> ("::" <literal>)
<call> = <identifier> "(" (<expr> ",")* ")"
<unary> = "(" <expr> ")" / "-"? <access> / <call>
<factor> = <unary> (("*" / "/") <unary>)*
<term> = <factor> (("+" / "-") <factor>)*
<equality> = <term> (("==" / "!=") <term>)*
<compare> = <equality> (("<" / ">") <equality>)*
<and> = <compare> ("&&" <compare>)*
<or> = <and> ("||" <and>)*
<assign> = <or> ("=" <or>)?
<expr> = <assign>
```

#### Statements:
```bnf
<comment> = "#" ... "#"
<program> = <import>* <function-decl>+
<import> = "import" <identifier> ";"
<function-decl> = "func" <identifier> "(" <arg-list> ")" <type-specifier> <block>
<arg-list> = (<arg> ",")*
<arg> = <identifier> ":" <type-specifier>
<type-specifier> = "bool" / "int" / "float"
<block> = "{" <nestable-stmt>+ "}"
<nestable-stmt> = <variable-decl> / <expr-stmt> / <return> / <if>
<variable-decl> = ("let" / "const") <identifier> ":" <type-specifier> "=" <expr> ";"
<expr-stmt> = <expr> ";"
<return> = "return" <expr> ";"
<if> = "if" "(" <expr> ")" <block>
```
