%skeleton "lalr1.cc"

%defines
%define api.namespace {yy}
%define parser_class_name {CParser}
%code requires {
	#include <iostream>
    #include <QtDebug>
    #include "CMouvement.h"
	
	using namespace std;
	
	class CScanner;
}

%lex-param		{ CScanner &scanner }
%parse-param	{ CScanner &scanner }

%code {
    static QList<CMouvement *> sequence;
    static bool inSequence = false;
    static QList<CMouvement *> result;

    static int yylex(yy::CParser::semantic_type *yylval, CScanner &scanner);
}
%union {
    CMouvement *mouvement;
    int repete;
}
%token <repete> DIGIT
%token <mouvement> MOUV
%token PRIME
%token PARO PARF

%start AXIOME
%%
AXIOME		:	EXP						{}
			;
DEBSEQ      :   PARO                    { inSequence = true; sequence.clear(); }
            ;
EXP			:	MVT						{}
            |   DEBSEQ EXP PARF DIGIT	{ for(int i=0;i<$4;i++) { for(int j=0;j<sequence.size();j++) { result << sequence.at(j); } } sequence.clear(); inSequence = false; }
            |   EXP EXP                 {}
			;
MVT			:	MOUV					{ if(inSequence) { sequence << $1; } else { result << $1; } }
            |	MOUV PRIME				{ $1->setInverse(true); if(inSequence) { sequence << $1; } else { result << $1; } }
            |	MOUV DIGIT				{ for(int i=0;i<$2;i++) if(inSequence) { sequence << $1; } else { result << $1; } }
            |	MOUV PRIME DIGIT		{ $1->setInverse(true); for(int i=0;i<$3;i++) if(inSequence) { sequence << $1; } else { result << $1; } }
			;
%%

void yy::CParser::error(const string &errMessage) {
    qDebug() << "Error : " << errMessage.c_str();
}

#include "CScanner.h"
static int yylex(yy::CParser::semantic_type *yylval, CScanner &scanner) {
	return scanner.yylex(yylval);
}

QList<CMouvement *> getResult(void) {
    return result;
}
