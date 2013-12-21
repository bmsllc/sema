
//
// sema - semaphore program
// accquire and release named semaphore for program synchronation
//	Questions...
/*
:! flex alex.l
:! gcc -c -g -o lex.yy.o lex.yy.c
:! g++ -c -o sema % 
:! g++ -g -o sema -pthread % lex.yy.o  -lfl
*/


#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>

using namespace std;

#define	PGM_NAME		"Sema"
#define	PGM_VERSION		"1.0"
#define	OPTION_STRING	"?Bc:n:uv"
#define	PATH_MAX		50

// command line parameters...
//  -B put dummy command in clipboard
//	-c command
//	-n semaphore name
//	-v verbose flag

#define	NAME_MAX 		50

#define	FALSE					0
#define	TRUE					1

void dummy( void  );
void usage( void );
void parseCommand( void );

char * lexString = NULL;

string	str;
char *	args[ 20 ];		// 20 max args


// foutput to stdout unless a filename is given;
FILE *	fout = NULL;

char	fname[ PATH_MAX ];

char	fn[ PATH_MAX ];
char *	name;
char *	command = NULL;
key_t	key;
int		semflg;
int		nsems;
int		semid;
bool	nomore = false;

int		verbose = 4;
#define	VERBOSE 	1
#define	PROLOGUE	2
#define	DEBUG 		4	// not used...


char * pgm = NULL;

int
main( int argc, char * argv[] )
{
	int	rc = 0;
	int	ch;
	int	pid;
	char * s;
	char   si;

	pid = getpid();

	fout = stdout;

	memset( fn, 0, PATH_MAX );
	pgm = argv[ 0 ];

	while((ch = getopt(argc,argv, OPTION_STRING )) != -1) {
		if( verbose ) { cout << "ch : " << ch << "\n"; }
		switch( ch ) {

      default:	//  '?' usually
        //fprintf(stderr, "%s: invalid option -%c\n\n", pgm, ch);
		opterr = 1;
        usage();
        exit(1);
        break;

      case '?': 						// tool diameter
	  	usage();
		exit(0);
		break;

      case 'B': 						// stuff clipboard
	  	dummy();
		exit( 0 );
		break;

      case 'c': // command line
		if( verbose ) { cout << "c : " << optarg << "\n"; }
          command = optarg;				// save command line
        break;

      case 'n': // semaphore name
		if( verbose ) { cout << "n : " << optarg << "\n"; }
          name = optarg;				// semaphore name
        break;

      case 'u': // unlink
		nomore = true;
        break;

      case 'v': // verbose
		verbose |= VERBOSE;
        break;
		}
	}

if( verbose ) { cout << "after getopt " << "\n"; }
if( name == NULL ) {
cout << "no semaphore name after getopt " << "\n";
}

	// always need the actual semaphore
	sem_t *	semID = sem_open(  name, O_CREAT | O_RDWR,  S_IRWXU | S_IRWXU | S_IRWXO, 1 );
	if( semID == SEM_FAILED ) {
		cerr << "Sem open failed..." << "\n";
		cerr << "error code: " << errno << "\t" << "\n" ;
		perror( "sema_open" );
		exit( -1 );
	}

	if( command != NULL ) {		// execute the command
		int	status = 0;
		int	endingStatus = 0;
		pid_t newP = (pid_t) NULL;
		lexString = command;
		parseCommand();

		sem_wait( semID );
		cout << pid << ": got it!" << "\n";

		newP = fork();					// fork new process, parent waits for completion
		if( newP < 0 ) {	// error
			sem_post( semID );		// release semaphore since we're quiting
			cerr << "Fork failed..." << "\n";
			perror( "fork error: " );
			exit( -1 );
		} else if( newP == 0 ) {	// child
			// exec the command line...
			cout << "The cmd: " << str << "\n";
			exit(0);
		} else {	// parent
			pid_t p = wait( & status );			// wait for the kid to finish, report ending status
			if( p == newP ) {					// status is for the child
				if( status == 0 ) {					// normal (expected ) return
				} else {							// some status returned.
					int	exitVal = WIFEXITED( status );	// basic exit status
					if( exitVal != 0 ) {
					endingStatus = WEXITSTATUS( status );		// exit status from child
					}
				}
			} else {							// not expected to happen
			}
		}

		sem_post( semID );		// parent waits
	}

	if( nomore == true ) {		// remove semaphore
		int rc = sem_unlink( name );
		if( rc < 0 ) {
			cerr << "Could not unlink the " << name << " semaphore." << "\n";
			cerr << "error code: " << errno << "\n";
		}
	}

	sem_close( semID );
	return rc;
}

//
// usage - give helpful information about this pogram
// 

void
usage() {
	fprintf( stderr, "%s - %s\n", PGM_NAME, PGM_VERSION );
	fprintf( stderr, "%s - {%s}\n", pgm, OPTION_STRING );
	fprintf( stderr, "\n\n" );
	fprintf( stderr, "\t%s - %s\n", "?", "Ask for help." );
	fprintf( stderr, "\t%s - %s\n", "B","Put dummy command line in clipboard, for pasting....");
	fprintf( stderr, "\t%s - %s\n", "n","Job name." );
	fprintf( stderr, "\t%s - %s\n", "v","Be verbose." );
	fprintf( stderr, "\n\n" );
}

void
dummy( ) {
FILE *	cb;

	cb = fopen( "/dev/clipboard", "w" );
	if( cb == NULL ) {
		fprintf( stderr, "Error openinmg clipboard...\n\n" );
	}
	// #define	OPTION_STRING	"?Bc:d:j:l:n:o:s:t:vw:"
	fprintf( cb, "%s -n ", pgm );
	fclose( cb );
}


// error Exit - we're here because of an error. don't leave any bad files....
void
errorExit( int e ) {
	if( fout != NULL ) {
		fclose(fout );
	}
}


// setup lexer and do the command line

extern "C" int	yylex( void );
extern "C" char *	yytext;

void
parseCommand( void ) {
int	val;
int	arg = 0;

	lexString = command;
	while( val = yylex() ) {
		cout << val << yytext << "\n";
		if( arg == 0 ) {
			cout << "Pgm is " << yytext << "\n";
		}
	}
}


extern "C" {

void
errorLexer( ) {
	cerr << "Lex error - prob too few chars returned." << "\nl";
	exit( -1 );
}

}



