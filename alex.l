%{
#define	BASE		300
#define	SOPTION 	300
#define	LOPTION 	301
#define	QSTRING		302
#define	STRING		303
#define	NUMBER		304


#include <stdio.h>
#include <unistd.h>
#include <limits.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#undef YY_INPUT

extern char * lexString;
extern void		errorLexer( void );

#define YY_INPUT( b, r, ms )( r = myInput( b, ms))

int myInput( char * buf, int max_size ) {

	int	l = strlen( lexString );
	int	n = MIN( max_size, l );

	if( n > 0 ) {
		memcpy( buf, lexString, n );
		lexString += n;
	}
	return n;
#if 0
	if( * lexString != 0 ) {			
		if( max_size != 1 ) {			
			errorLexer();				
		} else {						
			*buf = *lexString++;		
		}								
			result = 1;					
	} else {							
		result = 0;					
	}
#endif
}


%}
%%
[\t ]+			;

-[a-zA-Z0-9]{2,}			{ return LOPTION;	}
-[a-zA-Z0-9]+				{ return SOPTION;	}
\".*\"			{ return QSTRING;	}
[0-9]+			{ return NUMBER;	}
[-a-zA-z0-9_/\.\?]+	{ return STRING;	}

%%

#ifdef TEST
#include <stdio.h>

char *	tokens[] = { "SOPTION", "LOPTION", "QSTRING", "STRING", "NUMBER" };

int
main( int argc, char * argv[] ) 
{
int	val;

	while( val = yylex() ) {
		printf( "%8s :\t %s\n", tokens[ val - BASE ], yytext );
	}

	return 0;
}
#endif

