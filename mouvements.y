%skeleton "lalr1.cc"

%defines
%define api.namespace {yy}
%define api.parser.class {CParser}
%code requires {
	#include <iostream>
    #include <QtDebug>
    #include <QList>
    #include "CMouvement.h"
	
	using namespace std;
	
	class CScanner;
}

%lex-param		{ CScanner &scanner }
%parse-param	{ CScanner &scanner }

%code {
    static QList<CMouvement *> result;

    static int yylex(yy::CParser::semantic_type *yylval, CScanner &scanner);

    // Répète nb fois la liste ; chaque élément étant libéré par l'appelant, les répétitions sont des copies
    static QList<CMouvement *> *repete(QList<CMouvement *> *liste, int nb) {
        int taille = liste->size();

        if(nb <= 0) {
            qDeleteAll(*liste);
            liste->clear();
        }

        for(int i=1;i<nb;i++) {
            for(int j=0;j<taille;j++) {
                liste->append(new CMouvement(*liste->at(j)));
            }
        }

        return liste;
    }
}
%union {
    CMouvement *mouvement;
    QList<CMouvement *> *liste;
    int repete;
}
%token <repete> DIGIT
%token <mouvement> MOUV
%token PRIME
%token PARO PARF

%type <mouvement> MVT
%type <liste> LISTE ELEMENT
%type <repete> REPETITION

// Libère les valeurs abandonnées en cas d'erreur de syntaxe
%destructor { delete $$; } <mouvement>
%destructor { qDeleteAll(*$$); delete $$; } <liste>

%start AXIOME
%%
AXIOME		:	LISTE					{ result = *$1; delete $1; }
			;
LISTE		:	%empty					{ $$ = new QList<CMouvement *>(); }
			|	LISTE ELEMENT			{ $$ = $1; $$->append(*$2); delete $2; }
			;
ELEMENT		:	MVT REPETITION			{ $$ = repete(new QList<CMouvement *>({ $1 }), $2); }
			|	PARO LISTE PARF REPETITION	{ $$ = repete($2, $4); }
			;
REPETITION	:	%empty					{ $$ = 1; }
			|	DIGIT					{ $$ = $1; }
			;
MVT			:	MOUV					{ $$ = $1; }
			|	MOUV PRIME				{ $$ = $1; $$->setInverse(true); }
			;
%%

void yy::CParser::error(const string &errMessage) {
    qDebug() << "Error : " << errMessage.c_str();

    // L'axiome a pu être réduit avant la détection de l'erreur (ex. "RU)") : une commande invalide n'exécute rien
    qDeleteAll(result);
    result.clear();
}

#include "CScanner.h"
static int yylex(yy::CParser::semantic_type *yylval, CScanner &scanner) {
	return scanner.yylex(yylval);
}

QList<CMouvement *> getResult(void) {
    return result;
}

void clearResult(void) {
    result.clear();
}
