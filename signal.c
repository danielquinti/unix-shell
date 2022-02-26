#include <signal.h>
#include <string.h>


// Signals
struct SEN{
	char *name;
	int signal;
};
static struct SEN sigstrnum[]={
{"HUP", SIGHUP},
{"INT", SIGINT},
{"QUIT", SIGQUIT},
{"ILL", SIGILL},
{"TRAP", SIGTRAP},
{"ABRT", SIGABRT},
{"IOT", SIGIOT},
{"BUS", SIGBUS},
{"FPE", SIGFPE},
{"KILL", SIGKILL},
{"USR1", SIGUSR1},
{"SEGV", SIGSEGV},
{"USR2", SIGUSR2},
{"PIPE", SIGPIPE},
{"ALRM", SIGALRM},
{"TERM", SIGTERM},
{"CHLD", SIGCHLD},
{"CONT", SIGCONT},
{"STOP", SIGSTOP},
{"TSTP", SIGTSTP},
{"TTIN", SIGTTIN},
{"TTOU", SIGTTOU},
{"URG", SIGURG},
{"XCPU", SIGXCPU},
{"XFSZ", SIGXFSZ},
{"VTALRM", SIGVTALRM},
{"PROF", SIGPROF},
{"WINCH", SIGWINCH},
{"IO", SIGIO},
{"SYS", SIGSYS},

/*senales que no hay en todas partes*/
#ifdef SIGPOLL
{"POLL", SIGPOLL},
#endif
#ifdef SIGPWR
{"PWR", SIGPWR},
#endif
#ifdef SIGEMT
{"EMT", SIGEMT},
#endif
#ifdef SIGINFO
{"INFO", SIGINFO},
#endif
#ifdef SIGSTKFLT
{"STKFLT", SIGSTKFLT},
#endif
#ifdef SIGCLD
{"CLD", SIGCLD},
#endif
#ifdef SIGLOST
{"LOST", SIGLOST},
#endif
#ifdef SIGCANCEL
{"CANCEL", SIGCANCEL},
#endif
#ifdef SIGTHAW
{"THAW", SIGTHAW},
#endif
#ifdef SIGFREEZE
{"FREEZE", SIGFREEZE},
#endif
#ifdef SIGLWP
{"LWP", SIGLWP},
#endif
#ifdef SIGWAITING
{"WAITING", SIGWAITING},
#endif
{NULL,-1},
};    /*fin array sigstrnum */



// Returns the signal number from the name
int signal_number(char * sig) {
	for (int i = 0; sigstrnum[i].name != NULL; i++)
	if (!strcmp(sig, sigstrnum[i].name))
	return sigstrnum[i].signal;
	return -1;
}

char *signal_name(int sig) { /*devuelve el nombre senal a partir de la senal*/
                       /* para sitios donde no hay sig2str*/
	for (int i = 0; sigstrnum[i].name!=NULL; i++)
		if (sig==sigstrnum[i].signal)
	return sigstrnum[i].name;
	return ("SIGUNKNOWN");
}
