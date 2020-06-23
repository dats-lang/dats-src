%{
#include <stdio.h>
#include <stdint.h>

extern FILE *yyin;
extern int yylex();
extern int yyparse();
extern uint32_t dats_line;

int yyerror(const char *s);
%}

%token D_BEG 
%token D_END
%token K_NL
%token V1_NL V2_NL V4_NL V8_NL
%token EOL
%token SP

%start file_struct

%%

file_struct : beginning note D_END end_line
	    ;

note : K_NL SP note_length end_line
     | K_NL SP note_length end_line note
     ;

end_line : EOL
	 | EOL end_line
	 ;

beginning : with_or_without_sp_endl D_BEG with_or_without_sp_endl
	  ;

with_or_without_sp_endl :
		   | SP with_or_without_sp_endl
		   | EOL with_or_without_sp_endl
		   ;

note_length : V1_NL {printf("found note1 at line %d\n", dats_line);}
            | V2_NL {printf("found note2 at line %d\n", dats_line);}
            | V4_NL {printf("found note4 at line %d\n", dats_line);}
            | V8_NL {printf("found note8 at line %d\n", dats_line);}
            ;                                                 
%%

int main(int argc, char *argv[]){

   if (argc < 2) {
      fprintf(stderr, "ye may try %s [filename]\n", argv[0]);
      yyin = stdin;
      goto dats_parse;
      
   }
   if (!(yyin = fopen(argv[1], "r"))){
      perror(argv[1]);
      return 1;
   }
   dats_parse:
   yyparse();

   return 0;
}

int yyerror(const char *s){
   fprintf(stderr, "yyerror %s at line %d\n", s, dats_line-1);
   return 1;
}

