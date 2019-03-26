%{
#include <iostream>
#include <QtDebug>
#include "CScanner.h"

typedef yy::CParser::token token;

using namespace std;
%}

%option yyclass="CScanner"
%option noyywrap
%option c++

DIGIT			[0-9]
MOUV			[UDRLFBMES]
%%
{DIGIT}		{	sscanf(yytext, "%d", &yylval->repete);
                return token::DIGIT;
			}
{MOUV}		{	char c;
                sscanf(yytext, "%c", &c);
                yylval->mouvement = new CMouvement(c);
                return token::MOUV;
			}
"("			{	return token::PARO;
			}
")"			{	return token::PARF;
			}
"'"			{	return token::PRIME;
			}
.			{ 	std::cout << "Unkown lexem " <<  yytext;
			}
%%
