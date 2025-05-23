#ifndef AST_H
#define AST_H

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV
} ASTBinaryOp;

typedef enum {
    TYPE_UNKNOWN, // unkown first since we default to zero
    TYPE_DOUBLE,
    TYPE_STRING,
    TYPE_ERROR
} TypeKind;

typedef struct {
    TypeKind kind;
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
    AST_WHILE_LOOP
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    TypeInfo type_info;
    unsigned int type_var_id;  // Index in constraint system
    union {
        double number;
        char* string;
        struct {
            struct ASTNode *left;
            struct ASTNode *right;
            ASTBinaryOp op;
        } binary_op;
        struct {
            char *name;
            char **args;
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
void free_ast(ASTNode *node);

#endif
