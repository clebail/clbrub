#ifndef CSCANNER_H
#define CSCANNER_H

#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#undef  YY_DECL
#define YY_DECL int CScanner::yylex()
 
#include "mouvements.y.hpp"

class CScanner : public yyFlexLexer {
public:
    CScanner(std::istream *in) : yyFlexLexer(in), yylval(nullptr) {}
		
    int yylex(yy::CParser::semantic_type *lval) {
		yylval=lval;
		return yylex();
	}
private:
	int yylex();
    yy::CParser::semantic_type *yylval;
    QList<CMouvement *> result;
};		
#endif //CSCANNER_H
