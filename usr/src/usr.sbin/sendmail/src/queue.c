/*
 * Copyright (c) 1983 Eric P. Allman
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

# include "sendmail.h"

#ifndef lint
#ifdef QUEUE
static char sccsid[] = "@(#)queue.c	5.36 (Berkeley) %G% (with queueing)";
#else
static char sccsid[] = "@(#)queue.c	5.36 (Berkeley) %G% (without queueing)";
#endif
#endif /* not lint */

# include <sys/stat.h>
# include <sys/dir.h>
# include <sys/file.h>
# include <signal.h>
# include <errno.h>
# include <pwd.h>

# ifdef QUEUE

# ifdef LOCKF
# include <unistd.h>
# endif

/*
**  Work queue.
*/

struct work
{
	char		*w_name;	/* name of control file */
	long		w_pri;		/* priority of message, see below */
	time_t		w_ctime;	/* creation time of message */
	struct work	*w_next;	/* next in queue */
};

typedef struct work	WORK;

WORK	*WorkQ;			/* queue of things to be done */
/*
**  QUEUEUP -- queue a message up for future transmission.
**
**	Parameters:
**		e -- the envelope to queue up.
**		queueall -- if TRUE, queue all addresses, rather than
**			just those with the QQUEUEUP flag set.
**		announce -- if TRUE, tell when you are queueing up.
**
**	Returns:
**		none.
**
**	Side Effects:
**		The current request are saved in a control file.
**		The queue file is left locked.
*/

queueup(e, queueall, announce)
	register ENVELOPE *e;
	bool queueall;
	bool announce;
{
	char *qf;
	register FILE *tfp;
	register HDR *h;
	register ADDRESS *q;
	int fd;
	int i;
	bool newid;
	MAILER nullmailer;
	char buf[MAXLINE], tf[MAXLINE];

	/*
	**  Create control file.
	*/

	newid = (e->e_id == NULL);
	strcpy(tf, queuename(e, 't'));
	tfp = e->e_lockfp;
	if (tfp == NULL)
		newid = FALSE;
	if (newid)
	{
		tfp = e->e_lockfp;
	}
	else
	{
		/* get a locked tf file */
		for (i = 100; --i >= 0; )
		{
			fd = open(tf, O_CREAT|O_WRONLY|O_EXCL, FileMode);
			if (fd < 0)
			{
				if (errno == EEXIST)
					continue;
				syserr("queueup: cannot create temp file %s", tf);
				return;
			}
# ifdef LOCKF
			if (lockf(fd, F_TLOCK, 0) >= 0)
				break;
			if (errno != EACCES && errno != EAGAIN)
				syserr("cannot lockf(%s)", tf);
# else
			if (flock(fd, LOCK_EX|LOCK_NB) >= 0)
				break;
			if (errno != EWOULDBLOCK)
				syserr("cannot flock(%s)", tf);
# endif
			close(fd);
		}

		tfp = fdopen(fd, "w");
	}

	if (tTd(40, 1))
		printf("queueing %s\n", e->e_id);

	/*
	**  If there is no data file yet, create one.
	*/

	if (e->e_df == NULL)
	{
		register FILE *dfp;
		extern putbody();

		e->e_df = newstr(queuename(e, 'd'));
		fd = open(e->e_df, O_WRONLY|O_CREAT, FileMode);
		if (fd < 0)
		{
			syserr("queueup: cannot create %s", e->e_df);
			if (!newid)
				(void) fclose(tfp);
			return;
		}
		dfp = fdopen(fd, "w");
		(*e->e_putbody)(dfp, ProgMailer, e);
		(void) fclose(dfp);
		e->e_putbody = putbody;
	}

	/*
	**  Output future work requests.
	**	Priority and creation time should be first, since
	**	they are required by orderq.
	*/

	/* output message priority */
	fprintf(tfp, "P%ld\n", e->e_msgpriority);

	/* output creation time */
	fprintf(tfp, "T%ld\n", e->e_ctime);

	/* output name of data file */
	fprintf(tfp, "D%s\n", e->e_df);

	/* message from envelope, if it exists */
	if (e->e_message != NULL)
		fprintf(tfp, "M%s\n", e->e_message);

	/* output name of sender */
	fprintf(tfp, "S%s\n", e->e_from.q_paddr);

	/* output list of recipient addresses */
	for (q = e->e_sendqueue; q != NULL; q = q->q_next)
	{
		if (queueall ? !bitset(QDONTSEND|QSENT, q->q_flags) :
			       bitset(QQUEUEUP, q->q_flags))
		{
			char *ctluser, *getctluser();

			if ((ctluser = getctluser(q)) != NULL)
				fprintf(tfp, "C%s\n", ctluser);
			fprintf(tfp, "R%s\n", q->q_paddr);
			if (announce)
			{
				e->e_to = q->q_paddr;
				message(Arpa_Info, "queued");
				if (LogLevel > 4)
					logdelivery("queued");
				e->e_to = NULL;
			}
			if (tTd(40, 1))
			{
				printf("queueing ");
				printaddr(q, FALSE);
			}
		}
	}

	/* output list of error recipients */
	for (q = e->e_errorqueue; q != NULL; q = q->q_next)
	{
		if (!bitset(QDONTSEND, q->q_flags))
		{
			char *ctluser, *getctluser();

			if ((ctluser = getctluser(q)) != NULL)
				fprintf(tfp, "C%s\n", ctluser);
			fprintf(tfp, "E%s\n", q->q_paddr);
		}
	}

	/*
	**  Output headers for this message.
	**	Expand macros completely here.  Queue run will deal with
	**	everything as absolute headers.
	**		All headers that must be relative to the recipient
	**		can be cracked later.
	**	We set up a "null mailer" -- i.e., a mailer that will have
	**	no effect on the addresses as they are output.
	*/

	bzero((char *) &nullmailer, sizeof nullmailer);
	nullmailer.m_r_rwset = nullmailer.m_s_rwset = -1;
	nullmailer.m_eol = "\n";

	define('g', "\001f", e);
	for (h = e->e_header; h != NULL; h = h->h_link)
	{
		extern bool bitzerop();

		/* don't output null headers */
		if (h->h_value == NULL || h->h_value[0] == '\0')
			continue;

		/* don't output resent headers on non-resent messages */
		if (bitset(H_RESENT, h->h_flags) && !bitset(EF_RESENT, e->e_flags))
			continue;

		/* output this header */
		fprintf(tfp, "H");

		/* if conditional, output the set of conditions */
		if (!bitzerop(h->h_mflags) && bitset(H_CHECK|H_ACHECK, h->h_flags))
		{
			int j;

			(void) putc('?', tfp);
			for (j = '\0'; j <= '\177'; j++)
				if (bitnset(j, h->h_mflags))
					(void) putc(j, tfp);
			(void) putc('?', tfp);
		}

		/* output the header: expand macros, convert addresses */
		if (bitset(H_DEFAULT, h->h_flags))
		{
			(void) expand(h->h_value, buf, &buf[sizeof buf], e);
			fprintf(tfp, "%s: %s\n", h->h_field, buf);
		}
		else if (bitset(H_FROM|H_RCPT, h->h_flags))
		{
			commaize(h, h->h_value, tfp, bitset(EF_OLDSTYLE, e->e_flags),
				 &nullmailer);
		}
		else
			fprintf(tfp, "%s: %s\n", h->h_field, h->h_value);
	}

	/*
	**  Clean up.
	*/

	if (!newid)
	{
		qf = queuename(e, 'q');
		if (rename(tf, qf) < 0)
			syserr("cannot rename(%s, %s), df=%s", tf, qf, e->e_df);
		if (e->e_lockfp != NULL)
			(void) fclose(e->e_lockfp);
		e->e_lockfp = tfp;
	}
	else
		qf = tf;
	errno = 0;

# ifdef LOG
	/* save log info */
	if (LogLevel > 15)
		syslog(LOG_DEBUG, "%s: queueup, qf=%s, df=%s\n", e->e_id, qf, e->e_df);
# endif LOG
	fflush(tfp);
	return;
}
/*
**  RUNQUEUE -- run the jobs in the queue.
**
**	Gets the stuff out of the queue in some presumably logical
**	order and processes them.
**
**	Parameters:
**		forkflag -- TRUE if the queue scanning should be done in
**			a child process.  We double-fork so it is not our
**			child and we don't have to clean up after it.
**
**	Returns:
**		none.
**
**	Side Effects:
**		runs things in the mail queue.
*/

runqueue(forkflag)
	bool forkflag;
{
	extern bool shouldqueue();

	/*
	**  If no work will ever be selected, don't even bother reading
	**  the queue.
	*/

	CurrentLA = getla();	/* get load average */

	if (shouldqueue(-100000000L))
	{
		if (Verbose)
			printf("Skipping queue run -- load average too high\n");

		if (forkflag)
			return;
		finis();
	}

	/*
	**  See if we want to go off and do other useful work.
	*/

	if (forkflag)
	{
		int pid;

		pid = dofork();
		if (pid != 0)
		{
			extern void reapchild();

			/* parent -- pick up intermediate zombie */
#ifndef SIGCHLD
			(void) waitfor(pid);
#else SIGCHLD
			(void) signal(SIGCHLD, reapchild);
#endif SIGCHLD
			if (QueueIntvl != 0)
				(void) setevent(QueueIntvl, runqueue, TRUE);
			return;
		}
		/* child -- double fork */
#ifndef SIGCHLD
		if (fork() != 0)
			exit(EX_OK);
#else SIGCHLD
		(void) signal(SIGCHLD, SIG_DFL);
#endif SIGCHLD
	}

	setproctitle("running queue: %s", QueueDir);

# ifdef LOG
	if (LogLevel > 11)
		syslog(LOG_DEBUG, "runqueue %s, pid=%d", QueueDir, getpid());
# endif LOG

	/*
	**  Release any resources used by the daemon code.
	*/

# ifdef DAEMON
	clrdaemon();
# endif DAEMON

	/*
	**  Make sure the alias database is open.
	*/

	initaliases(AliasFile, FALSE);

	/*
	**  Start making passes through the queue.
	**	First, read and sort the entire queue.
	**	Then, process the work in that order.
	**		But if you take too long, start over.
	*/

	/* order the existing work requests */
	(void) orderq(FALSE);

	/* process them once at a time */
	while (WorkQ != NULL)
	{
		WORK *w = WorkQ;

		WorkQ = WorkQ->w_next;
		dowork(w);
		free(w->w_name);
		free((char *) w);
	}

	/* exit without the usual cleanup */
	exit(ExitStat);
}
/*
**  ORDERQ -- order the work queue.
**
**	Parameters:
**		doall -- if set, include everything in the queue (even
**			the jobs that cannot be run because the load
**			average is too high).  Otherwise, exclude those
**			jobs.
**
**	Returns:
**		The number of request in the queue (not necessarily
**		the number of requests in WorkQ however).
**
**	Side Effects:
**		Sets WorkQ to the queue of available work, in order.
*/

# define NEED_P		001
# define NEED_T		002

# ifndef DIR
# define DIR		FILE
# define direct		dir
# define opendir(d)	fopen(d, "r")
# define readdir(f)	((fread(&dbuf, sizeof dbuf, 1, f) > 0) ? &dbuf : 0)
static struct dir	dbuf;
# define closedir(f)	fclose(f)
# endif DIR

orderq(doall)
	bool doall;
{
	register struct direct *d;
	register WORK *w;
	DIR *f;
	register int i;
	WORK wlist[QUEUESIZE+1];
	int wn = -1;
	extern workcmpf();

	/* clear out old WorkQ */
	for (w = WorkQ; w != NULL; )
	{
		register WORK *nw = w->w_next;

		WorkQ = nw;
		free(w->w_name);
		free((char *) w);
		w = nw;
	}

	/* open the queue directory */
	f = opendir(".");
	if (f == NULL)
	{
		syserr("orderq: cannot open \"%s\" as \".\"", QueueDir);
		return (0);
	}

	/*
	**  Read the work directory.
	*/

	while ((d = readdir(f)) != NULL)
	{
		FILE *cf;
		char lbuf[MAXNAME];

		/* is this an interesting entry? */
		if (d->d_ino == 0)
			continue;
# ifdef DEBUG
		if (tTd(40, 10))
			printf("orderq: %12s\n", d->d_name);
# endif DEBUG
		if (d->d_name[0] != 'q' || d->d_name[1] != 'f')
			continue;

		/* yes -- open control file (if not too many files) */
		if (++wn >= QUEUESIZE)
			continue;
		cf = fopen(d->d_name, "r");
		if (cf == NULL)
		{
			/* this may be some random person sending hir msgs */
			/* syserr("orderq: cannot open %s", cbuf); */
			if (tTd(41, 2))
				printf("orderq: cannot open %s (%d)\n",
					d->d_name, errno);
			errno = 0;
			wn--;
			continue;
		}
		w = &wlist[wn];
		w->w_name = newstr(d->d_name);

		/* make sure jobs in creation don't clog queue */
		w->w_pri = 0x7fffffff;
		w->w_ctime = 0;

		/* extract useful information */
		i = NEED_P | NEED_T;
		while (i != 0 && fgets(lbuf, sizeof lbuf, cf) != NULL)
		{
			extern long atol();

			switch (lbuf[0])
			{
			  case 'P':
				w->w_pri = atol(&lbuf[1]);
				i &= ~NEED_P;
				break;

			  case 'T':
				w->w_ctime = atol(&lbuf[1]);
				i &= ~NEED_T;
				break;
			}
		}
		(void) fclose(cf);

		if (!doall && shouldqueue(w->w_pri))
		{
			/* don't even bother sorting this job in */
			wn--;
		}
	}
	(void) closedir(f);
	wn++;

	/*
	**  Sort the work directory.
	*/

	qsort((char *) wlist, min(wn, QUEUESIZE), sizeof *wlist, workcmpf);

	/*
	**  Convert the work list into canonical form.
	**	Should be turning it into a list of envelopes here perhaps.
	*/

	WorkQ = NULL;
	for (i = min(wn, QUEUESIZE); --i >= 0; )
	{
		w = (WORK *) xalloc(sizeof *w);
		w->w_name = wlist[i].w_name;
		w->w_pri = wlist[i].w_pri;
		w->w_ctime = wlist[i].w_ctime;
		w->w_next = WorkQ;
		WorkQ = w;
	}

	if (tTd(40, 1))
	{
		for (w = WorkQ; w != NULL; w = w->w_next)
			printf("%32s: pri=%ld\n", w->w_name, w->w_pri);
	}

	return (wn);
}
/*
**  WORKCMPF -- compare function for ordering work.
**
**	Parameters:
**		a -- the first argument.
**		b -- the second argument.
**
**	Returns:
**		-1 if a < b
**		 0 if a == b
**		+1 if a > b
**
**	Side Effects:
**		none.
*/

workcmpf(a, b)
	register WORK *a;
	register WORK *b;
{
	long pa = a->w_pri + a->w_ctime;
	long pb = b->w_pri + b->w_ctime;

	if (pa == pb)
		return (0);
	else if (pa > pb)
		return (1);
	else
		return (-1);
}
/*
**  DOWORK -- do a work request.
**
**	Parameters:
**		w -- the work request to be satisfied.
**
**	Returns:
**		none.
**
**	Side Effects:
**		The work request is satisfied if possible.
*/

dowork(w)
	register WORK *w;
{
	register int i;
	extern bool shouldqueue();
	extern bool readqf();

	if (tTd(40, 1))
		printf("dowork: %s pri %ld\n", w->w_name, w->w_pri);

	/*
	**  Ignore jobs that are too expensive for the moment.
	*/

	if (shouldqueue(w->w_pri))
	{
		if (Verbose)
			printf("\nSkipping %s\n", w->w_name + 2);
		return;
	}

	/*
	**  Fork for work.
	*/

	if (ForkQueueRuns)
	{
		i = fork();
		if (i < 0)
		{
			syserr("dowork: cannot fork");
			return;
		}
	}
	else
	{
		i = 0;
	}

	if (i == 0)
	{
		/*
		**  CHILD
		**	Lock the control file to avoid duplicate deliveries.
		**		Then run the file as though we had just read it.
		**	We save an idea of the temporary name so we
		**		can recover on interrupt.
		*/

		/* set basic modes, etc. */
		(void) alarm(0);
		clearenvelope(CurEnv, FALSE);
		QueueRun = TRUE;
		ErrorMode = EM_MAIL;
		CurEnv->e_id = &w->w_name[2];
# ifdef LOG
		if (LogLevel > 11)
			syslog(LOG_DEBUG, "%s: dowork, pid=%d", CurEnv->e_id,
			       getpid());
# endif LOG

		/* don't use the headers from sendmail.cf... */
		CurEnv->e_header = NULL;

		/* read the queue control file -- return if locked */
		if (!readqf(CurEnv))
		{
			if (ForkQueueRuns)
				exit(EX_OK);
			else
				return;
		}

		CurEnv->e_flags |= EF_INQUEUE;
		eatheader(CurEnv);

		/* do the delivery */
		if (!bitset(EF_FATALERRS, CurEnv->e_flags))
			sendall(CurEnv, SM_DELIVER);

		/* finish up and exit */
		if (ForkQueueRuns)
			finis();
		else
			dropenvelope(CurEnv);
	}
	else
	{
		/*
		**  Parent -- pick up results.
		*/

		errno = 0;
		(void) waitfor(i);
	}
}
/*
**  READQF -- read queue file and set up environment.
**
**	Parameters:
**		e -- the envelope of the job to run.
**
**	Returns:
**		TRUE if it successfully read the queue file.
**		FALSE otherwise.
**
**	Side Effects:
**		The queue file is returned locked.
*/

# ifdef LOCKF
# define RDLK_MODE	"r+"
# else
# define RDLK_MODE	"r"
# endif

bool
readqf(e)
	register ENVELOPE *e;
{
	char *qf;
	register FILE *qfp;
	char buf[MAXFIELD];
	extern char *fgetfolded();
	extern long atol();
	int gotctluser = 0;
	int fd;
	extern ADDRESS *sendto();

	/*
	**  Read and process the file.
	*/

	qf = queuename(e, 'q');
	qfp = fopen(qf, RDLK_MODE);
	if (qfp == NULL)
	{
		if (errno != ENOENT)
			syserr("readqf: no control file %s", qf);
		return FALSE;
	}

# ifdef LOCKF
	if (lockf(fileno(qfp), F_TLOCK, 0) < 0)
# else
	if (flock(fileno(qfp), LOCK_EX|LOCK_NB) < 0)
# endif
	{
		/* being processed by another queuer */
		if (Verbose)
			printf("%s: locked\n", CurEnv->e_id);
# ifdef LOG
		if (LogLevel > 9)
			syslog(LOG_DEBUG, "%s: locked", CurEnv->e_id);
# endif LOG
		(void) fclose(qfp);
		return FALSE;
	}

	/* save this lock */
	e->e_lockfp = qfp;

	/* do basic system initialization */
	initsys();

	FileName = qf;
	LineNumber = 0;
	if (Verbose)
		printf("\nRunning %s\n", e->e_id);
	while (fgetfolded(buf, sizeof buf, qfp) != NULL)
	{
		if (tTd(40, 4))
			printf("+++++ %s\n", buf);
		switch (buf[0])
		{
		  case 'C':		/* specify controlling user */
			setctluser(&buf[1]);
			gotctluser = 1;
			break;

		  case 'R':		/* specify recipient */
			(void) sendto(&buf[1], 1, (ADDRESS *) NULL, 0);
			break;

		  case 'E':		/* specify error recipient */
			sendtolist(&buf[1], (ADDRESS *) NULL, &e->e_errorqueue);
			break;

		  case 'H':		/* header */
			(void) chompheader(&buf[1], FALSE);
			break;

		  case 'M':		/* message */
			e->e_message = newstr(&buf[1]);
			break;

		  case 'S':		/* sender */
			setsender(newstr(&buf[1]));
			break;

		  case 'D':		/* data file name */
			e->e_df = newstr(&buf[1]);
			e->e_dfp = fopen(e->e_df, "r");
			if (e->e_dfp == NULL)
				syserr("readqf: cannot open %s", e->e_df);
			break;

		  case 'T':		/* init time */
			e->e_ctime = atol(&buf[1]);
			break;

		  case 'P':		/* message priority */
			e->e_msgpriority = atol(&buf[1]) + WkTimeFact;
			break;

		  case '\0':		/* blank line; ignore */
			break;

		  default:
			syserr("readqf(%s:%d): bad line \"%s\"", e->e_id,
				LineNumber, buf);
			break;
		}
		/*
		**  The `C' queue file command operates on the next line,
		**  so we use "gotctluser" to maintain state as follows:
		**      0 - no controlling user,
		**      1 - controlling user has been set but not used,
		**      2 - controlling user must be used on next iteration.
		*/
		if (gotctluser == 1)
			gotctluser++;
		else if (gotctluser == 2)
		{
			clrctluser();
			gotctluser = 0;
		}
	}

	/* clear controlling user in case we break out prematurely */
	clrctluser();

	FileName = NULL;

	/*
	**  If we haven't read any lines, this queue file is empty.
	**  Arrange to remove it without referencing any null pointers.
	*/

	if (LineNumber == 0)
	{
		errno = 0;
		e->e_flags |= EF_CLRQUEUE | EF_FATALERRS | EF_RESPONSE;
	}
	return TRUE;
}
/*
**  PRINTQUEUE -- print out a representation of the mail queue
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Prints a listing of the mail queue on the standard output.
*/

printqueue()
{
	register WORK *w;
	FILE *f;
	int nrequests;
	char buf[MAXLINE];
	char cbuf[MAXLINE];

	/*
	**  Read and order the queue.
	*/

	nrequests = orderq(TRUE);

	/*
	**  Print the work list that we have read.
	*/

	/* first see if there is anything */
	if (nrequests <= 0)
	{
		printf("Mail queue is empty\n");
		return;
	}

	CurrentLA = getla();	/* get load average */

	printf("\t\tMail Queue (%d request%s", nrequests, nrequests == 1 ? "" : "s");
	if (nrequests > QUEUESIZE)
		printf(", only %d printed", QUEUESIZE);
	if (Verbose)
		printf(")\n--QID-- --Size-- -Priority- ---Q-Time--- -----------Sender/Recipient-----------\n");
	else
		printf(")\n--QID-- --Size-- -----Q-Time----- ------------Sender/Recipient------------\n");
	for (w = WorkQ; w != NULL; w = w->w_next)
	{
		struct stat st;
		auto time_t submittime = 0;
		long dfsize = -1;
		char message[MAXLINE];
		extern bool shouldqueue();

		f = fopen(w->w_name, "r");
		if (f == NULL)
		{
			errno = 0;
			continue;
		}
		printf("%7s", w->w_name + 2);
# ifdef LOCKF
		if (lockf(fileno(f), F_TEST, 0) < 0)
# else
		if (flock(fileno(f), LOCK_SH|LOCK_NB) < 0)
# endif
			printf("*");
		else if (shouldqueue(w->w_pri))
			printf("X");
		else
			printf(" ");
		errno = 0;

		message[0] = '\0';
		cbuf[0] = '\0';
		while (fgets(buf, sizeof buf, f) != NULL)
		{
			fixcrlf(buf, TRUE);
			switch (buf[0])
			{
			  case 'M':	/* error message */
				(void) strcpy(message, &buf[1]);
				break;

			  case 'S':	/* sender name */
				if (Verbose)
					printf("%8ld %10ld %.12s %.38s", dfsize,
					    w->w_pri, ctime(&submittime) + 4,
					    &buf[1]);
				else
					printf("%8ld %.16s %.45s", dfsize,
					    ctime(&submittime), &buf[1]);
				if (message[0] != '\0')
					printf("\n\t\t (%.60s)", message);
				break;

			  case 'C':	/* controlling user */
				if (strlen(buf) < MAXLINE-3)	/* sanity */
					(void) strcat(buf, ") ");
				cbuf[0] = cbuf[1] = '(';
				(void) strncpy(&cbuf[2], &buf[1], MAXLINE-1);
				cbuf[MAXLINE-1] = '\0';
				break;

			  case 'R':	/* recipient name */
				if (cbuf[0] != '\0') {
					/* prepend controlling user to `buf' */
					(void) strncat(cbuf, &buf[1],
					              MAXLINE-strlen(cbuf));
					cbuf[MAXLINE-1] = '\0';
					(void) strcpy(buf, cbuf);
					cbuf[0] = '\0';
				}
				if (Verbose)
					printf("\n\t\t\t\t\t %.38s", &buf[1]);
				else
					printf("\n\t\t\t\t  %.45s", &buf[1]);
				break;

			  case 'T':	/* creation time */
				submittime = atol(&buf[1]);
				break;

			  case 'D':	/* data file name */
				if (stat(&buf[1], &st) >= 0)
					dfsize = st.st_size;
				break;
			}
		}
		if (submittime == (time_t) 0)
			printf(" (no control file)");
		printf("\n");
		(void) fclose(f);
	}
}

# endif QUEUE
/*
**  QUEUENAME -- build a file name in the queue directory for this envelope.
**
**	Assigns an id code if one does not already exist.
**	This code is very careful to avoid trashing existing files
**	under any circumstances.
**
**	Parameters:
**		e -- envelope to build it in/from.
**		type -- the file type, used as the first character
**			of the file name.
**
**	Returns:
**		a pointer to the new file name (in a static buffer).
**
**	Side Effects:
**		If no id code is already assigned, queuename will
**		assign an id code, create a qf file, and leave a
**		locked, open-for-write file pointer in the envelope.
*/

char *
queuename(e, type)
	register ENVELOPE *e;
	char type;
{
	static char buf[MAXNAME];
	static int pid = -1;
	char c1 = 'A';
	char c2 = 'A';

	if (e->e_id == NULL)
	{
		char qf[20];

		/* find a unique id */
		if (pid != getpid())
		{
			/* new process -- start back at "AA" */
			pid = getpid();
			c1 = 'A';
			c2 = 'A' - 1;
		}
		(void) sprintf(qf, "qfAA%05d", pid);

		while (c1 < '~' || c2 < 'Z')
		{
			int i;

			if (c2 >= 'Z')
			{
				c1++;
				c2 = 'A' - 1;
			}
			qf[2] = c1;
			qf[3] = ++c2;
			if (tTd(7, 20))
				printf("queuename: trying \"%s\"\n", qf);

			i = open(qf, O_WRONLY|O_CREAT|O_EXCL, FileMode);
			if (i < 0)
			{
				if (errno == EEXIST)
					continue;
				syserr("queuename: Cannot create \"%s\" in \"%s\"",
					qf, QueueDir);
				exit(EX_UNAVAILABLE);
			}
# ifdef LOCKF
			if (lockf(i, F_TLOCK, 0) >= 0)
# else
			if (flock(i, LOCK_EX|LOCK_NB) >= 0)
# endif
			{
				e->e_lockfp = fdopen(i, "w");
				break;
			}

			/* a reader got the file; abandon it and try again */
			(void) close(i);
		}
		if (c1 >= '~' && c2 >= 'Z')
		{
			syserr("queuename: Cannot create \"%s\" in \"%s\"",
				qf, QueueDir);
			exit(EX_OSERR);
		}
		e->e_id = newstr(&qf[2]);
		define('i', e->e_id, e);
		if (tTd(7, 1))
			printf("queuename: assigned id %s, env=%x\n", e->e_id, e);
# ifdef LOG
		if (LogLevel > 16)
			syslog(LOG_DEBUG, "%s: assigned id", e->e_id);
# endif LOG
	}

	if (type == '\0')
		return (NULL);
	(void) sprintf(buf, "%cf%s", type, e->e_id);
	if (tTd(7, 2))
		printf("queuename: %s\n", buf);
	return (buf);
}
/*
**  UNLOCKQUEUE -- unlock the queue entry for a specified envelope
**
**	Parameters:
**		e -- the envelope to unlock.
**
**	Returns:
**		none
**
**	Side Effects:
**		unlocks the queue for `e'.
*/

unlockqueue(e)
	ENVELOPE *e;
{
	/* if there is a lock file in the envelope, close it */
	if (e->e_lockfp != NULL)
		fclose(e->e_lockfp);
	e->e_lockfp = NULL;

	/* remove the transcript */
# ifdef LOG
	if (LogLevel > 19)
		syslog(LOG_DEBUG, "%s: unlock", e->e_id);
# endif LOG
	if (!tTd(51, 4))
		xunlink(queuename(e, 'x'));

}
/*
**  GETCTLUSER -- return controlling user if mailing to prog or file
**
**	Check for a "|" or "/" at the beginning of the address.  If
**	found, return a controlling username.
**
**	Parameters:
**		a - the address to check out
**
**	Returns:
**		Either NULL, if we werent mailing to a program or file,
**		or a controlling user name (possibly in getpwuid's
**		static buffer).
**
**	Side Effects:
**		none.
*/

char *
getctluser(a)
	ADDRESS *a;
{
	extern ADDRESS *getctladdr();
	struct passwd *pw;
	char *retstr;

	/*
	**  Get unquoted user for file, program or user.name check.
	**  N.B. remove this code block to always emit controlling
	**  addresses (at the expense of backward compatibility).
	*/

	{
		char buf[MAXNAME];
		(void) strncpy(buf, a->q_paddr, MAXNAME);
		buf[MAXNAME-1] = '\0';
		stripquotes(buf, TRUE);

		if (buf[0] != '|' && buf[0] != '/')
			return((char *)NULL);
	}

	a = getctladdr(a);		/* find controlling address */

	if (a != NULL && a->q_uid != 0 && (pw = getpwuid(a->q_uid)) != NULL)
		retstr = pw->pw_name;
	else				/* use default user */
		retstr = DefUser;

	if (tTd(40, 5))
		printf("Set controlling user for `%s' to `%s'\n",
		       (a == NULL)? "<null>": a->q_paddr, retstr);

	return(retstr);
}
/*
**  SETCTLUSER - sets `CtlUser' to controlling user
**  CLRCTLUSER - clears controlling user (no params, nothing returned)
**
**	These routines manipulate `CtlUser'.
**
**	Parameters:
**		str  - controlling user as passed to setctluser()
**
**	Returns:
**		None.
**
**	Side Effects:
**		`CtlUser' is changed.
*/

static char CtlUser[MAXNAME];

setctluser(str)
register char *str;
{
	(void) strncpy(CtlUser, str, MAXNAME);
	CtlUser[MAXNAME-1] = '\0';
}

clrctluser()
{
	CtlUser[0] = '\0';
}

/*
**  SETCTLADDR -- create a controlling address
**
**	If global variable `CtlUser' is set and we are given a valid
**	address, make that address a controlling address; change the
**	`q_uid', `q_gid', and `q_ruser' fields and set QGOODUID.
**
**	Parameters:
**		a - address for which control uid/gid info may apply
**
**	Returns:
**		None.	
**
**	Side Effects:
**		Fills in uid/gid fields in address and sets QGOODUID
**		flag if appropriate.
*/

setctladdr(a)
	ADDRESS *a;
{
	struct passwd *pw;

	/*
	**  If there is no current controlling user, or we were passed a
	**  NULL addr ptr or we already have a controlling user, return.
	*/

	if (CtlUser[0] == '\0' || a == NULL || a->q_ruser)
		return;

	/*
	**  Set up addr fields for controlling user.  If `CtlUser' is no
	**  longer valid, use the default user/group.
	*/

	if ((pw = getpwnam(CtlUser)) != NULL)
	{
		if (a->q_home)
			free(a->q_home);
		a->q_home = newstr(pw->pw_dir);
		a->q_uid = pw->pw_uid;
		a->q_gid = pw->pw_gid;
		a->q_ruser = newstr(CtlUser);
	}
	else
	{
		a->q_uid = DefUid;
		a->q_gid = DefGid;
		a->q_ruser = newstr(DefUser);
	}

	a->q_flags |= QGOODUID;		/* flag as a "ctladdr"  */

	if (tTd(40, 5))
		printf("Restored controlling user for `%s' to `%s'\n",
		       a->q_paddr, a->q_ruser);
}
