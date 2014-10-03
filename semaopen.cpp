//
// semaopen -  a semaphore program
// accquire and release named semaphore - just a test prograam....
//	Questions...
/*
:! g++ -c -o sema % 
:! g++ -g -o semaopen -pthread % 
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

#define SIMPLE			1					// non zero to keep things simple

#define	PGM_NAME		"SemaOpen"
#define	PGM_VERSION		"1.0"
#define	OPTION_STRING	"?n:uv"
#define	PATH_MAX		200

// command line parameters...
//	-n semaphore name
//	-u unlink semaphore on exit
//	-v verbose flag

#define	NAME_MAX 		50

#define	FALSE					0
#define	TRUE					1

void dummy( void  );
void usage( void );
void parseCommand( void );

char * lexString = NULL;

string	str;
char *	thePgm = NULL;
char	thePath[ PATH_MAX ];

char *	args[ 30 ];		// 30 max args


char	fname[ PATH_MAX ];

char	fn[ PATH_MAX ];
char	reply[ PATH_MAX ];
char *	name = NULL;
char *	command = NULL;
key_t	key;
int		semflg;
int		nsems;
int		semid;
bool	nomore = false;

int		verbose = 0;
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

	memset( fn, 0, PATH_MAX );
	pgm = argv[ 0 ];

	while((ch = getopt(argc,argv, OPTION_STRING )) != -1) {
		switch( ch ) {

      default:	//  '?' usually
		opterr = 1;
        usage();
        exit(1);
        break;

      case '?': 						// tool diameter
	  	usage();
		exit(0);
		break;

      case 'n': 						// semaphore name
          name = optarg;
        break;

      case 'u': 						// unlink
		nomore = true;
        break;

      case 'v': // verbose
		verbose |= VERBOSE;
        break;
		}
	}

	if( name == NULL ) {
		cerr << "No name specified for the semaphone" << endl ;
		exit( -1 );
	}

	// always need the actual semaphore
	int	oflag = O_CREAT | O_RDWR;
	mode_t	mode = S_IRWXU | S_IRWXG | S_IRWXO;
	//mode_t	mode = S_IRWXU;
	int	status;
	int	value = 1;	
	sem_t *	semID = NULL;

	struct stat sb;
	char	devsem[ 30 ];
	strcpy( devsem, "/dev/shm/sem." );
	strcat( devsem, name );					// result: /dev/shm/sem.name
#if SIMPLE
	cout << "will be creating..." << endl;
	semID = sem_open(  name, oflag,  mode, value );		// create the semaphore
#else
	cout << "Stat on the semaphore: " << devsem << endl;
	if ( stat( devsem, &sb) == -1) {
		cout << "will be creating..." << endl;
		semID = sem_open(  name, oflag,  mode, value );		// create the semaphore
    } else {
		cout << "will be opening..." << endl;
		oflag = O_RDWR;
		semID = sem_open(  name, oflag );				// do not create the semaphore
	}
#endif

	if( semID == SEM_FAILED ) {
		cerr << "Sem open failed: " << name << "(" << devsem << ")" 
				<< ", oflag: 0x" << hex  << oflag  
				<< oct << ", mode: 0" << mode 
				<< ", value: " << dec << value << endl ;
		cerr << "error code: " << dec << errno << "\t"  ;
		perror( "sema_open" );
		exit( -1 );
	}

	//clog << "Sem open: " << name << ", oflag: " << hex  << oflag  << ", mode: 0" << oct << mode << dec << endl ;
	int	semvalue;
	errno = 0;
	int sv = sem_getvalue( semID, &semvalue);
	cout << pid << ": sv is: " << sv << ", semvalue is 0x" << hex << semvalue << dec << endl ;
	perror( "sem_getvalue: ") ;
	cout << "Hit enter to end this program...." << endl;
	cin >> reply;
	cout << "Reply: " << reply << endl;

	if( nomore == true ) {		// remove semaphore
		cout << "Removing the semaphore: " << name << endl;
		int urc = sem_unlink( name );
		if( urc < 0 ) {
			cerr << "Could not unlink the " << name << " semaphore." << "\n";
			cerr << "error code: " << errno << "\n";
		}
	}

	rc = sem_close( semID );
	cout << "Closing the semaphore: " << devsem << " rc: " << rc << endl;
	//clog << "Closing the semaphore: " << devsem << endl;
	return 0;
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
	fprintf( stderr, "\t%s - %s\n", "n","semaphore name." );
	fprintf( stderr, "\t%s - %s\n", "u","unlink semaphone on exit." );
	fprintf( stderr, "\t%s - %s\n", "v","Be verbose." );
	fprintf( stderr, "\n\n" );
}




