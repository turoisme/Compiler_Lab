/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdlib.h>

#include "reader.h"
#include "scanner.h"
#include "parser.h"
#include "error.h"

Token *currentToken;
Token *lookAhead;

void scan(void) {
  Token* tmp = currentToken;
  currentToken = lookAhead;
  lookAhead = getValidToken();
  free(tmp);
  
}

void eat(TokenType tokenType) {
  if (lookAhead->tokenType == tokenType) {
    printToken(lookAhead);
    scan();
  } else missingToken(tokenType, lookAhead->lineNo, lookAhead->colNo);
}

void compileProgram(void) {
  assert("Parsing a Program ....");
  eat(KW_PROGRAM);
  eat(TK_IDENT);
  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_PERIOD);
  assert("Program parsed!");
}

void compileBlock(void) {
  assert("Parsing a Block ....");
  if (lookAhead->tokenType == KW_CONST) {
    eat(KW_CONST);
    compileConstDecl();
    compileConstDecls();
    compileBlock2();
  } 
  else compileBlock2();
  assert("Block parsed!");
}

void compileBlock2(void) {
  if (lookAhead->tokenType == KW_TYPE) {
    eat(KW_TYPE);
    compileTypeDecl();
    compileTypeDecls();
    compileBlock3();
  } 
  else compileBlock3();
}

void compileBlock3(void) {
  if (lookAhead->tokenType == KW_VAR) {
    eat(KW_VAR);
    compileVarDecl();
    compileVarDecls();
    compileBlock4();
  } 
  else compileBlock4();
}

void compileBlock4(void) {
  compileSubDecls();
  compileBlock5();
}

void compileBlock5(void) {
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
}

void compileConstDecls(void) {
  if (lookAhead->tokenType == SB_SEMICOLON) {
    eat(SB_SEMICOLON);
    if (lookAhead->tokenType == TK_IDENT) {
      compileConstDecl();
      compileConstDecls();
    }
  }
}

void compileConstDecl(void) {
  eat(TK_IDENT);
  eat(SB_EQ);
  compileConstant();
}

void compileTypeDecls(void) {
  if (lookAhead->tokenType == SB_SEMICOLON) {
    eat(SB_SEMICOLON);
    if (lookAhead->tokenType == TK_IDENT) {
      compileTypeDecl();
      compileTypeDecls();
    }
  }
}

void compileTypeDecl(void) {
  eat(TK_IDENT);
  eat(SB_EQ);
  compileType();
}

void compileVarDecls(void) {
  if (lookAhead->tokenType == SB_SEMICOLON) {
    eat(SB_SEMICOLON);
    if (lookAhead->tokenType == TK_IDENT) {
      compileVarDecl();
      compileVarDecls();
    }
  }
}

void compileVarDecl(void) {
  eat(TK_IDENT);
  eat(SB_COLON);
  compileType();
}

void compileSubDecls(void) {
  assert("Parsing subtoutines ....");
  if ((lookAhead->tokenType == KW_FUNCTION) || (lookAhead->tokenType == KW_PROCEDURE)) {
    if (lookAhead->tokenType == KW_FUNCTION) 
      compileFuncDecl();
    else 
      compileProcDecl();
    compileSubDecls();
  }
  assert("Subtoutines parsed ....");
}

void compileFuncDecl(void) {
  assert("Parsing a function ....");
  eat(KW_FUNCTION);
  eat(TK_IDENT);
  compileParams();
  eat(SB_COLON);
  compileBasicType();
  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_SEMICOLON);
  assert("Function parsed ....");
}

void compileProcDecl(void) {
  assert("Parsing a procedure ....");
  eat(KW_PROCEDURE);
  eat(TK_IDENT);
  compileParams();
  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_SEMICOLON);
  assert("Procedure parsed ....");
}

void compileUnsignedConstant(void) {
  if (lookAhead->tokenType == TK_NUMBER) 
    eat(TK_NUMBER);
  else if (lookAhead->tokenType == TK_IDENT)
    eat(TK_IDENT);
  else if (lookAhead->tokenType == TK_CHAR)
    eat(TK_CHAR);
  else 
    error(ERR_INVALIDCONSTANT, lookAhead->lineNo, lookAhead->colNo);
}

void compileConstant(void) {
  if (lookAhead->tokenType == SB_PLUS) {
    eat(SB_PLUS);
    compileUnsignedConstant();
  } else if (lookAhead->tokenType == SB_MINUS) {
    eat(SB_MINUS);
    compileUnsignedConstant();
  } else 
    compileUnsignedConstant();
}

void compileConstant2(void) {
  if (lookAhead->tokenType == SB_PLUS) {
    eat(SB_PLUS);
    compileUnsignedConstant();
  } else if (lookAhead->tokenType == SB_MINUS) {
    eat(SB_MINUS);
    compileUnsignedConstant();
  }
}

void compileType(void) {
  if (lookAhead->tokenType == TK_IDENT) {
    eat(TK_IDENT);
  } else if (lookAhead->tokenType == KW_ARRAY) {
    eat(KW_ARRAY);
    eat(SB_LSEL);
    compileUnsignedConstant();
    eat(SB_RSEL);
    eat(KW_OF);
    compileType();
  } else {
    compileBasicType();
  }
}

void compileBasicType(void) {
  switch(lookAhead->tokenType) {
  case KW_INTEGER:
    eat(KW_INTEGER);
    break;
  case KW_CHAR:
    eat(KW_CHAR);
    break;
  default:
    error(ERR_INVALIDTYPE, lookAhead->lineNo, lookAhead->colNo);
  }
}

void compileParams(void) {
  if (lookAhead->tokenType == SB_LPAR) {
    eat(SB_LPAR);
    compileParam();
    compileParams2();
    eat(SB_RPAR);
  }
}

void compileParams2(void) {
  while (lookAhead->tokenType == SB_SEMICOLON) {
    eat(SB_SEMICOLON);
    compileParam();
  }
}

void compileParam(void) {
  eat(TK_IDENT);
  eat(SB_COLON);
  compileType();
}

int isStatement(TokenType tokenType) {
  return tokenType == TK_IDENT || tokenType == KW_CALL || tokenType == KW_BEGIN ||
         tokenType == KW_IF || tokenType == KW_WHILE || tokenType == KW_FOR;
}

void compileStatements(void) {
  compileStatement();
  compileStatements2();
}

void compileStatements2(void) {
  while (lookAhead->tokenType == SB_SEMICOLON || isStatement(lookAhead->tokenType)) {
    if (lookAhead->tokenType != SB_SEMICOLON) {
      eat(SB_SEMICOLON); 
    } else {
      eat(SB_SEMICOLON);
      if (isStatement(lookAhead->tokenType)) {
        compileStatement();
      }
    }
  }
}

void compileStatement(void) {
  switch (lookAhead->tokenType) {
  case TK_IDENT:
    compileAssignSt();
    break;
  case KW_CALL:
    compileCallSt();
    break;
  case KW_BEGIN:
    compileGroupSt();
    break;
  case KW_IF:
    compileIfSt();
    break;
  case KW_WHILE:
    compileWhileSt();
    break;
  case KW_FOR:
    compileForSt();
    break;
    // EmptySt needs to check FOLLOW tokens
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
    break;
    // Error occurs
  default:
    error(ERR_INVALIDSTATEMENT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

void compileAssignSt(void) {
  assert("Parsing an assign statement ....");
  eat(TK_IDENT);
  compileIndexes();
  eat(SB_ASSIGN);
  compileExpression();
  assert("Assign statement parsed ....");
}

void compileCallSt(void) {
  assert("Parsing a call statement ....");
  eat(KW_CALL);
  eat(TK_IDENT);
  compileArguments();
  assert("Call statement parsed ....");
}

void compileGroupSt(void) {
  assert("Parsing a group statement ....");
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
  assert("Group statement parsed ....");
}

void compileIfSt(void) {
  assert("Parsing an if statement ....");
  eat(KW_IF);
  compileCondition();
  eat(KW_THEN);
  compileStatement();
  if (lookAhead->tokenType == KW_ELSE) 
    compileElseSt();
  assert("If statement parsed ....");
}

void compileElseSt(void) {
  eat(KW_ELSE);
  compileStatement();
}

void compileWhileSt(void) {
  assert("Parsing a while statement ....");
  eat(KW_WHILE);
  compileCondition();
  eat(KW_DO);
  compileStatement();
  assert("While statement parsed ....");
}

void compileForSt(void) {
  assert("Parsing a for statement ....");
  eat(KW_FOR);
  eat(TK_IDENT);
  eat(SB_ASSIGN);
  compileExpression();
  eat(KW_TO);
  compileExpression();
  eat(KW_DO);
  compileStatement();
  assert("For statement parsed ....");
}

void compileArguments(void) {
  if (lookAhead->tokenType == SB_LPAR) {
    eat(SB_LPAR);
    compileExpression();
    compileArguments2();
    eat(SB_RPAR);
  }
}

void compileArguments2(void) {
  while (lookAhead->tokenType == SB_COMMA) {
    eat(SB_COMMA);
    compileExpression();
  }
}

void compileCondition(void) {
  compileExpression();
  switch(lookAhead->tokenType) {
  case SB_LT:
    eat(SB_LT);
    break;
  case SB_LE:
    eat(SB_LE);
    break;
  case SB_GT:
    eat(SB_GT);
    break;
  case SB_GE:
    eat(SB_GE);
    break;
  case SB_EQ:
    eat(SB_EQ);
    break;
  case SB_NEQ:
    eat(SB_NEQ);
    break;
  default:
    error(ERR_INVALIDCOMPARATOR, lookAhead->lineNo, lookAhead->colNo);
  }
  compileExpression();
}

void compileCondition2(void) {
  switch(lookAhead->tokenType) {
  case SB_LT:
    eat(SB_LT);
    break;
  case SB_LE:
    eat(SB_LE);
    break;
  case SB_GT:
    eat(SB_GT);
    break;
  case SB_GE:
    eat(SB_GE);
    break;
  case SB_EQ:
    eat(SB_EQ);
    break;
  case SB_NEQ:
    eat(SB_NEQ);
    break;
  }
}

void compileExpression(void) {
  assert("Parsing an expression");
  compileConstant2();
  compileTerm();
  compileExpression2();
  assert("Expression parsed");
}

void compileExpression2(void) {
  while ((lookAhead->tokenType == SB_PLUS) || (lookAhead->tokenType == SB_MINUS)) {
    if (lookAhead->tokenType == SB_PLUS)
      eat(SB_PLUS);
    else
      eat(SB_MINUS);
    compileTerm();
  }
}


void compileExpression3(void) {
  while ((lookAhead->tokenType == SB_TIMES) || (lookAhead->tokenType == SB_SLASH)) {
    if (lookAhead->tokenType == SB_TIMES)
      eat(SB_TIMES);
    else
      eat(SB_SLASH);
    compileFactor();
  }
}

void compileTerm(void) {
  compileFactor();
  compileExpression3();
}

void compileTerm2(void) {
  while ((lookAhead->tokenType == SB_TIMES) || (lookAhead->tokenType == SB_SLASH)) {
    if (lookAhead->tokenType == SB_TIMES)
      eat(SB_TIMES);
    else
      eat(SB_SLASH);
    compileFactor();
  }
}

void compileFactor(void) {
  switch(lookAhead->tokenType) {
  case TK_NUMBER:
    eat(TK_NUMBER);
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    compileIndexes();
    compileArguments();
    break;
  case TK_CHAR:
    eat(TK_CHAR);
    break;
  case SB_LPAR:
    eat(SB_LPAR);
    compileExpression();
    eat(SB_RPAR);
    break;
  default:
    error(ERR_INVALIDFACTOR, lookAhead->lineNo, lookAhead->colNo);
  }
}

void compileIndexes(void) {
  while (lookAhead->tokenType == SB_LSEL) {
    eat(SB_LSEL);
    compileExpression();
    eat(SB_RSEL);
  }
}

int compile(char *fileName) {
  if (openInputStream(fileName) == IO_ERROR)
    return IO_ERROR;

  currentToken = NULL;
  lookAhead = getValidToken();

  compileProgram();

  free(currentToken);
  free(lookAhead);
  closeInputStream();
  return IO_SUCCESS;

}

