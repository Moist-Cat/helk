#ifndef AST_H
#define AST_H

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_EXP,
    OP_MOD
} ASTBinaryOp;

typedef enum {
    TYPE_UNKNOWN, // unkown first since we default to zero
    TYPE_DOUBLE,
    TYPE_STRING,
    TYPE_TRIVAL, // for statements. In particular, function declarations
    TYPE_ERROR
} TypeKind;

typedef struct {
    TypeKind kind;
    char* name; // what to print
    char* cls; // ...
    unsigned int is_literal;
} TypeInfo;

typedef enum {
    AST_BLOCK,
    AST_NUMBER,
    AST_STRING,
    AST_BINARY_OP,
    AST_FUNCTION_DEF,
    AST_FUNCTION_CALL,
    AST_VARIABLE,
    AST_VARIABLE_DEF,
    AST_LET_IN,
    AST_CONDITIONAL,
    AST_WHILE_LOOP,
    AST_TYPE_DEF,
    AST_METHOD_DEF,
    AST_CONSTRUCTOR,
    AST_FIELD_DEF,
    AST_FIELD_ACCESS,
    AST_METHOD_CALL
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    TypeInfo type_info;
    unsigned int type_var_id;  // Index in constraint system
    union {
        double number;
        char* string;
        struct {
            char* name;
            char* base_type; // NULL if no inheritance
            struct ASTNode** fields; // Array of AST_VARIABLE_DEF nodes
            unsigned int field_count;
            struct ASTNode** methods; // Array of AST_FUNCTION_DEF nodes
            unsigned int method_count;
        } type_decl;
        struct {
            struct ASTNode *left;
            struct ASTNode *right;
            ASTBinaryOp op;
        } binary_op;
        struct {
            char *name;
            char **args; // backwards compatibility
            struct ASTNode **args_definitions;
            unsigned int arg_count;
            struct ASTNode *body;
        } function_def;
        struct {
            char *name;
            unsigned int arg_count;
            // array of nodes
            struct ASTNode **args;
        } function_call;
        struct {
            char *name;
        } variable;
        struct {
            char *name;
            struct ASTNode *body;
        } variable_def;
        struct {
            struct ASTNode **statements;
            unsigned int stmt_count;
        } block;
        struct {
            char **var_names;
            struct ASTNode **var_values;
            unsigned int var_count;
            struct ASTNode *body;
        } let_in;
        struct {
            struct ASTNode* hypothesis;
            struct ASTNode* thesis;
            struct ASTNode* antithesis;
        } conditional;
        struct {
            struct ASTNode* cond;
            struct ASTNode* body;
        } while_loop;
        struct {
            char* cls;
            struct ASTNode** args;
            unsigned int arg_count;
        } constructor;
        struct {
            char* cls;
            char* field;
            unsigned int pos; // added later
        } field_access;
        struct {
            char* name;
            struct ASTNode* default_value;
        } field_def;
        struct {
            struct ASTNode* cls;
            char* method;
            struct ASTNode** args;
            unsigned int arg_count;
        } method_call;
    };
} ASTNode;

ASTNode* create_ast_block(ASTNode **block, unsigned int stmt_count);
ASTNode *create_ast_string(char* ptr);
ASTNode* create_ast_number(double value);
ASTNode* create_ast_binary_op(ASTNode *left, ASTNode *right, ASTBinaryOp op);
ASTNode* create_ast_function_def(char *name, ASTNode *body, char **args, unsigned int arg_count);
ASTNode* create_ast_function_call(char *name, ASTNode **args, unsigned int arg_count);
ASTNode* create_ast_variable(char *name);
ASTNode* create_ast_variable_def(char *name, ASTNode *body);
ASTNode* create_ast_let_in(char **names, ASTNode **values, unsigned int count, ASTNode *body);
ASTNode* create_ast_conditional(ASTNode* hypothesis, ASTNode* thesis, ASTNode* antithesis);
ASTNode* create_ast_while_loop(ASTNode* cond, ASTNode* body);
ASTNode* create_ast_type_def(char* name, char* base_type, ASTNode** members, unsigned int member_count);
ASTNode* create_ast_constructor(char* cls, ASTNode** args, unsigned int arg_count);
ASTNode* create_ast_field_def(char* name, ASTNode* default_value);
ASTNode* create_ast_field_access(char* cls, char* field);
ASTNode* create_ast_method_call(ASTNode* cls, char* method, ASTNode** args, unsigned int arg_count);
void free_ast(ASTNode *node);

#endif
