/*
 * Copyright (c) 1992, Brian Berliner and Jeff Polk
 * Copyright (c) 1989-1992, Brian Berliner
 * 
 * You may distribute under the terms of the GNU General Public License as
 * specified in the README file that comes with the CVS 1.4 kit.
 * 
 * "update" updates the version in the present directory with respect to the RCS
 * repository.  The present version must have been created by "checkout". The
 * user can keep up-to-date by calling "update" whenever he feels like it.
 * 
 * The present version can be committed by "commit", but this keeps the version
 * in tact.
 * 
 * Arguments following the options are taken to be file names to be updated,
 * rather than updating the entire directory.
 * 
 * Modified or non-existent RCS files are checked out and reported as U
 * <user_file>
 * 
 * Modified user files are reported as M <user_file>.  If both the RCS file and
 * the user file have been modified, the user file is replaced by the result
 * of rcsmerge, and a backup file is written for the user in .#file.version.
 * If this throws up irreconcilable differences, the file is reported as C
 * <user_file>, and as M <user_file> otherwise.
 * 
 * Files added but not yet committed are reported as A <user_file>. Files
 * removed but not yet committed are reported as R <user_file>.
 * 
 * If the current directory contains subdirectories that hold concurrent
 * versions, these are updated too.  If the -d option was specified, new
 * directories added to the repository are automatically created and updated
 * as well.
 */

#include "cvs.h"
#ifdef SERVER_SUPPORT
#include "md5.h"
#endif
#include "watch.h"
#include "fileattr.h"
#include "edit.h"

static int checkout_file PROTO((char *file, char *repository, List *entries,
			  RCSNode *rcsnode, Vers_TS *vers_ts, char *update_dir));
#ifdef SERVER_SUPPORT
static int patch_file PROTO((char *file, char *repository, List *entries,
		       RCSNode*rcsnode, Vers_TS *vers_ts, char *update_dir,
		       int *docheckout, struct stat *file_info,
		       unsigned char *checksum));
#endif
static int isemptydir PROTO((char *dir));
static int merge_file PROTO((char *file, char *repository, List *entries,
		       Vers_TS *vers, char *update_dir));
static int scratch_file PROTO((char *file, char *repository, List * entries,
			 char *update_dir));
static Dtype update_dirent_proc PROTO((char *dir, char *repository, char *update_dir));
static int update_dirleave_proc PROTO((char *dir, int err, char *update_dir));
static int update_fileproc PROTO ((struct file_info *));
static int update_filesdone_proc PROTO((int err, char *repository,
					char *update_dir));
static int write_letter PROTO((char *file, int letter, char *update_dir));
#ifdef SERVER_SUPPORT
static void join_file PROTO((char *file, RCSNode *rcsnode, Vers_TS *vers_ts,
		       char *update_dir, List *entries, char *repository));
#else
static void join_file PROTO((char *file, RCSNode *rcsnode, Vers_TS *vers_ts,
		       char *update_dir, List *entries));
#endif

static char *options = NULL;
static char *tag = NULL;
static char *date = NULL;
static char *join_rev1, *date_rev1;
static char *join_rev2, *date_rev2;
static int aflag = 0;
static int force_tag_match = 1;
static int update_build_dirs = 0;
static int update_prune_dirs = 0;
static int pipeout = 0;
#ifdef SERVER_SUPPORT
static int patches = 0;
#endif
static List *ignlist = (List *) NULL;
static time_t last_register_time;
static const char *const update_usage[] =
{
    "Usage: %s %s [-APdflRp] [-k kopt] [-r rev|-D date] [-j rev]\n",
    "    [-I ign] [-W spec] [files...]\n",
    "\t-A\tReset any sticky tags/date/kopts.\n",
    "\t-P\tPrune empty directories.\n",
    "\t-d\tBuild directories, like checkout does.\n",
    "\t-f\tForce a head revision match if tag/date not found.\n",
    "\t-l\tLocal directory only, no recursion.\n",
    "\t-R\tProcess directories recursively.\n",
    "\t-p\tSend updates to standard output.\n",
    "\t-k kopt\tUse RCS kopt -k option on checkout.\n",
    "\t-r rev\tUpdate using specified revision/tag.\n",
    "\t-D date\tSet date to update from.\n",
    "\t-j rev\tMerge in changes made between current revision and rev.\n",
    "\t-I ign\tMore files to ignore (! to reset).\n",
    "\t-W spec\tWrappers specification line.\n",
    NULL
};

/*
 * update is the argv,argc based front end for arg parsing
 */
int
update (argc, argv)
    int argc;
    char **argv;
{
    int c, err;
    int local = 0;			/* recursive by default */
    int which;				/* where to look for files and dirs */

    if (argc == -1)
	usage (update_usage);

    ign_setup ();
    wrap_setup ();

    /* parse the args */
    optind = 1;
    while ((c = getopt (argc, argv, "ApPflRQqduk:r:D:j:I:W:")) != -1)
    {
	switch (c)
	{
	    case 'A':
		aflag = 1;
		break;
	    case 'I':
		ign_add (optarg, 0);
		break;
	    case 'W':
		wrap_add (optarg, 0);
		break;
	    case 'k':
		if (options)
		    free (options);
		options = RCS_check_kflag (optarg);
		break;
	    case 'l':
		local = 1;
		break;
	    case 'R':
		local = 0;
		break;
	    case 'Q':
	    case 'q':
#ifdef SERVER_SUPPORT
		/* The CVS 1.5 client sends these options (in addition to
		   Global_option requests), so we must ignore them.  */
		if (!server_active)
#endif
		    error (1, 0,
			   "-q or -Q must be specified before \"%s\"",
			   command_name);
		break;
	    case 'd':
		update_build_dirs = 1;
		break;
	    case 'f':
		force_tag_match = 0;
		break;
	    case 'r':
		tag = optarg;
		break;
	    case 'D':
		date = Make_Date (optarg);
		break;
	    case 'P':
		update_prune_dirs = 1;
		break;
	    case 'p':
		pipeout = 1;
		noexec = 1;		/* so no locks will be created */
		break;
	    case 'j':
		if (join_rev2)
		    error (1, 0, "only two -j options can be specified");
		if (join_rev1)
		    join_rev2 = optarg;
		else
		    join_rev1 = optarg;
		break;
	    case 'u':
#ifdef SERVER_SUPPORT
		if (server_active)
		    patches = 1;
		else
#endif
		    usage (update_usage);
		break;
	    case '?':
	    default:
		usage (update_usage);
		break;
	}
    }
    argc -= optind;
    argv += optind;

#ifdef CLIENT_SUPPORT
    if (client_active) 
    {
	/* The first pass does the regular update.  If we receive at least
	   one patch which failed, we do a second pass and just fetch
	   those files whose patches failed.  */
	do
	{
	    int status;

	    start_server ();

	    if (local)
		send_arg("-l");
	    if (update_build_dirs)
		send_arg("-d");
	    if (pipeout)
		send_arg("-p");
	    if (!force_tag_match)
		send_arg("-f");
	    if (aflag)
		send_arg("-A");
	    if (update_prune_dirs)
		send_arg("-P");
	    client_prune_dirs = update_prune_dirs;
	    option_with_arg ("-r", tag);
	    if (date)
		client_senddate (date);
	    if (join_rev1)
		option_with_arg ("-j", join_rev1);
	    if (join_rev2)
		option_with_arg ("-j", join_rev2);

	    /* If the server supports the command "update-patches", that means
	       that it knows how to handle the -u argument to update, which
	       means to send patches instead of complete files.  */
	    if (failed_patches == NULL)
	    {
		struct request *rq;

		for (rq = requests; rq->name != NULL; rq++)
		{
		    if (strcmp (rq->name, "update-patches") == 0)
		    {
			if (rq->status == rq_supported)
			{
			    send_arg("-u");
			}
			break;
		    }
		}
	    }

	    if (failed_patches == NULL)
	    {
		send_file_names (argc, argv, SEND_EXPAND_WILD);
		send_files (argc, argv, local, aflag);
	    }
	    else
	    {
		int i;

		(void) printf ("%s client: refetching unpatchable files\n",
			       program_name);

		if (toplevel_wd[0] != '\0'
		    && chdir (toplevel_wd) < 0)
		{
		    error (1, errno, "could not chdir to %s", toplevel_wd);
		}

		for (i = 0; i < failed_patches_count; i++)
		    (void) unlink_file (failed_patches[i]);
		send_file_names (failed_patches_count, failed_patches, 0);
		send_files (failed_patches_count, failed_patches, local,
			    aflag);
	    }

	    failed_patches = NULL;
	    failed_patches_count = 0;

	    send_to_server ("update\012", 0);

	    status = get_responses_and_close ();
	    if (status != 0)
		return status;

	} while (failed_patches != NULL);

	return 0;
    }
#endif

    if (tag != NULL)
	tag_check_valid (tag, argc, argv, local, aflag, "");
    /* FIXME: We don't call tag_check_valid on join_rev1 and join_rev2
       yet (make sure to handle ':' correctly if we do, though).  */

    /*
     * If we are updating the entire directory (for real) and building dirs
     * as we go, we make sure there is no static entries file and write the
     * tag file as appropriate
     */
    if (argc <= 0 && !pipeout)
    {
	if (update_build_dirs)
	{
	    if (unlink_file (CVSADM_ENTSTAT) < 0 && ! existence_error (errno))
		error (1, errno, "cannot remove file %s", CVSADM_ENTSTAT);
#ifdef SERVER_SUPPORT
	    if (server_active)
		server_clear_entstat (".", Name_Repository (NULL, NULL));
#endif
	}

	/* keep the CVS/Tag file current with the specified arguments */
	if (aflag || tag || date)
	{
	    WriteTag ((char *) NULL, tag, date);
#ifdef SERVER_SUPPORT
	    if (server_active)
		server_set_sticky (".", Name_Repository (NULL, NULL), tag, date);
#endif
	}
    }

    /* look for files/dirs locally and in the repository */
    which = W_LOCAL | W_REPOS;

    /* look in the attic too if a tag or date is specified */
    if (tag != NULL || date != NULL || joining())
	which |= W_ATTIC;

    /* call the command line interface */
    err = do_update (argc, argv, options, tag, date, force_tag_match,
		     local, update_build_dirs, aflag, update_prune_dirs,
		     pipeout, which, join_rev1, join_rev2, (char *) NULL);

    /* free the space Make_Date allocated if necessary */
    if (date != NULL)
	free (date);

    return (err);
}

/*
 * Command line interface to update (used by checkout)
 */
int
do_update (argc, argv, xoptions, xtag, xdate, xforce, local, xbuild, xaflag,
	   xprune, xpipeout, which, xjoin_rev1, xjoin_rev2, preload_update_dir)
    int argc;
    char **argv;
    char *xoptions;
    char *xtag;
    char *xdate;
    int xforce;
    int local;
    int xbuild;
    int xaflag;
    int xprune;
    int xpipeout;
    int which;
    char *xjoin_rev1;
    char *xjoin_rev2;
    char *preload_update_dir;
{
    int err = 0;
    char *cp;

    /* fill in the statics */
    options = xoptions;
    tag = xtag;
    date = xdate;
    force_tag_match = xforce;
    update_build_dirs = xbuild;
    aflag = xaflag;
    update_prune_dirs = xprune;
    pipeout = xpipeout;

    /* setup the join support */
    join_rev1 = xjoin_rev1;
    join_rev2 = xjoin_rev2;
    if (join_rev1 && (cp = strchr (join_rev1, ':')) != NULL)
    {
	*cp++ = '\0';
	date_rev1 = Make_Date (cp);
    }
    else
	date_rev1 = (char *) NULL;
    if (join_rev2 && (cp = strchr (join_rev2, ':')) != NULL)
    {
	*cp++ = '\0';
	date_rev2 = Make_Date (cp);
    }
    else
	date_rev2 = (char *) NULL;

    /* call the recursion processor */
    err = start_recursion (update_fileproc, update_filesdone_proc,
			   update_dirent_proc, update_dirleave_proc,
			   argc, argv, local, which, aflag, 1,
			   preload_update_dir, 1, 0);

    /* see if we need to sleep before returning */
    if (last_register_time)
    {
	time_t now;

	(void) time (&now);
	if (now == last_register_time)
	    sleep (1);			/* to avoid time-stamp races */
    }

    return (err);
}

/*
 * This is the callback proc for update.  It is called for each file in each
 * directory by the recursion code.  The current directory is the local
 * instantiation.  file is the file name we are to operate on. update_dir is
 * set to the path relative to where we started (for pretty printing).
 * repository is the repository. entries and srcfiles are the pre-parsed
 * entries and source control files.
 * 
 * This routine decides what needs to be done for each file and does the
 * appropriate magic for checkout
 */
static int
update_fileproc (finfo)
    struct file_info *finfo;
{
    int retval;
    Ctype status;
    Vers_TS *vers;

    status = Classify_File (finfo->file, tag, date, options, force_tag_match,
			    aflag, finfo->repository, finfo->entries, finfo->rcs, &vers,
			    finfo->update_dir, pipeout);
    if (pipeout)
    {
	/*
	 * We just return success without doing anything if any of the really
	 * funky cases occur
	 * 
	 * If there is still a valid RCS file, do a regular checkout type
	 * operation
	 */
	switch (status)
	{
	    case T_UNKNOWN:		/* unknown file was explicitly asked
					 * about */
	    case T_REMOVE_ENTRY:	/* needs to be un-registered */
	    case T_ADDED:		/* added but not committed */
		retval = 0;
		break;
	    case T_CONFLICT:		/* old punt-type errors */
		retval = 1;
		break;
	    case T_UPTODATE:		/* file was already up-to-date */
	    case T_NEEDS_MERGE:		/* needs merging */
	    case T_MODIFIED:		/* locally modified */
	    case T_REMOVED:		/* removed but not committed */
	    case T_CHECKOUT:		/* needs checkout */
#ifdef SERVER_SUPPORT
	    case T_PATCH:		/* needs patch */
#endif
		retval = checkout_file (finfo->file, finfo->repository, finfo->entries, finfo->rcs,
					vers, finfo->update_dir);
		break;

	    default:			/* can't ever happen :-) */
		error (0, 0,
		       "unknown file status %d for file %s", status, finfo->file);
		retval = 0;
		break;
	}
    }
    else
    {
	switch (status)
	{
	    case T_UNKNOWN:		/* unknown file was explicitly asked
					 * about */
	    case T_UPTODATE:		/* file was already up-to-date */
		retval = 0;
		break;
	    case T_CONFLICT:		/* old punt-type errors */
		retval = 1;
		(void) write_letter (finfo->file, 'C', finfo->update_dir);
		break;
	    case T_NEEDS_MERGE:		/* needs merging */
		if (noexec)
		{
		    retval = 1;
		    (void) write_letter (finfo->file, 'C', finfo->update_dir);
		}
		else
		{
		    if (wrap_merge_is_copy (finfo->file))
			/* Should we be warning the user that we are
			 * overwriting the user's copy of the file?  */
			retval = checkout_file (finfo->file, finfo->repository, finfo->entries,
						finfo->rcs, vers, finfo->update_dir);
		    else
			retval = merge_file (finfo->file, finfo->repository, finfo->entries,
					     vers, finfo->update_dir);
		}
		break;
	    case T_MODIFIED:		/* locally modified */
		retval = 0;
		if (vers->ts_conflict)
		{
		    char *filestamp;
		    int retcode;

		    /*
		     * If the timestamp has changed and no conflict indicators
		     * are found, it isn't a 'C' any more.
		     */
#ifdef SERVER_SUPPORT
		    if (server_active)
			retcode = vers->ts_conflict[0] != '=';
		    else {
			filestamp = time_stamp (finfo->file);
			retcode = strcmp (vers->ts_conflict, filestamp);
			free (filestamp);
		    }
#else
		    filestamp = time_stamp (finfo->file);
		    retcode = strcmp (vers->ts_conflict, filestamp);
		    free (filestamp);
#endif

		    if (retcode)
		    {
			/*
			 * If the timestamps differ, look for Conflict
			 * indicators to see if 'C' anyway.
			 */
			run_setup ("%s", GREP);
			run_arg (RCS_MERGE_PAT);
			run_arg (finfo->file);
			retcode = run_exec (RUN_TTY, DEVNULL,
					    RUN_TTY,RUN_NORMAL);
			if (retcode == -1)
			{
			    error (1, errno,
				"fork failed while examining conflict in `%s'",
				       finfo->fullname);
			}
		    }
		    if (!retcode)
		    {
			(void) write_letter (finfo->file, 'C', finfo->update_dir);
			retval = 1;
		    }
		    else
		    {
			/* Reregister to clear conflict flag. */
			Register (finfo->entries, finfo->file, vers->vn_rcs, vers->ts_rcs,
				  vers->options, vers->tag,
				  vers->date, (char *)0);
		    }
		}
		if (!retval)
		    retval = write_letter (finfo->file, 'M', finfo->update_dir);
		break;
#ifdef SERVER_SUPPORT
	    case T_PATCH:		/* needs patch */
		if (patches)
		{
		    int docheckout;
		    struct stat file_info;
		    unsigned char checksum[16];

		    retval = patch_file (finfo->file, finfo->repository, finfo->entries, finfo->rcs,
					 vers, finfo->update_dir, &docheckout,
					 &file_info, checksum);
		    if (! docheckout)
		    {
		        if (server_active && retval == 0)
			    server_updated (finfo->file, finfo->update_dir, finfo->repository,
					    SERVER_PATCHED, &file_info,
					    checksum);
			break;
		    }
		}
		/* Fall through.  */
		/* If we're not running as a server, just check the
		   file out.  It's simpler and faster than starting up
		   two new processes (diff and patch).  */
		/* Fall through.  */
#endif
	    case T_CHECKOUT:		/* needs checkout */
		retval = checkout_file (finfo->file, finfo->repository, finfo->entries, finfo->rcs,
					vers, finfo->update_dir);
#ifdef SERVER_SUPPORT
		if (server_active && retval == 0)
		    server_updated (finfo->file, finfo->update_dir, finfo->repository,
				    SERVER_UPDATED, (struct stat *) NULL,
				    (unsigned char *) NULL);
#endif
		break;
	    case T_ADDED:		/* added but not committed */
		retval = write_letter (finfo->file, 'A', finfo->update_dir);
		break;
	    case T_REMOVED:		/* removed but not committed */
		retval = write_letter (finfo->file, 'R', finfo->update_dir);
		break;
	    case T_REMOVE_ENTRY:	/* needs to be un-registered */
		retval = scratch_file (finfo->file, finfo->repository, finfo->entries, finfo->update_dir);
#ifdef SERVER_SUPPORT
		if (server_active && retval == 0)
		    server_updated (finfo->file, finfo->update_dir, finfo->repository,
				    SERVER_UPDATED, (struct stat *) NULL,
				    (unsigned char *) NULL);
#endif
		break;
	    default:			/* can't ever happen :-) */
		error (0, 0,
		       "unknown file status %d for file %s", status, finfo->file);
		retval = 0;
		break;
	}
    }

    /* only try to join if things have gone well thus far */
    if (retval == 0 && join_rev1)
#ifdef SERVER_SUPPORT
	join_file (finfo->file, finfo->rcs, vers, finfo->update_dir, finfo->entries, finfo->repository);
#else
	join_file (finfo->file, finfo->rcs, vers, finfo->update_dir, finfo->entries);
#endif

    /* if this directory has an ignore list, add this file to it */
    if (ignlist)
    {
	Node *p;

	p = getnode ();
	p->type = FILES;
	p->key = xstrdup (finfo->file);
	if (addnode (ignlist, p) != 0)
	    freenode (p);
    }

    freevers_ts (&vers);
    return (retval);
}

static void update_ignproc PROTO ((char *, char *));

static void
update_ignproc (file, dir)
    char *file;
    char *dir;
{
    (void) write_letter (file, '?', dir);
}

/* ARGSUSED */
static int
update_filesdone_proc (err, repository, update_dir)
    int err;
    char *repository;
    char *update_dir;
{
    /* if this directory has an ignore list, process it then free it */
    if (ignlist)
    {
	ignore_files (ignlist, update_dir, update_ignproc);
	dellist (&ignlist);
    }

    /* Clean up CVS admin dirs if we are export */
    if (strcmp (command_name, "export") == 0)
    {
	/* I'm not sure the existence_error is actually possible (except
	   in cases where we really should print a message), but since
	   this code used to ignore all errors, I'll play it safe.  */
	if (unlink_file_dir (CVSADM) < 0 && !existence_error (errno))
	    error (0, errno, "cannot remove %s directory", CVSADM);
    }
#ifdef SERVER_SUPPORT
    else if (!server_active && !pipeout)
#else
    else if (!pipeout)
#endif /* SERVER_SUPPORT */
    {
        /* If there is no CVS/Root file, add one */
        if (!isfile (CVSADM_ROOT))
	    Create_Root( (char *) NULL, CVSroot );
    }

    return (err);
}

/*
 * update_dirent_proc () is called back by the recursion processor before a
 * sub-directory is processed for update.  In this case, update_dirent proc
 * will probably create the directory unless -d isn't specified and this is a
 * new directory.  A return code of 0 indicates the directory should be
 * processed by the recursion code.  A return of non-zero indicates the
 * recursion code should skip this directory.
 */
static Dtype
update_dirent_proc (dir, repository, update_dir)
    char *dir;
    char *repository;
    char *update_dir;
{
    if (ignore_directory (update_dir))
      {
	/* print the warm fuzzy message */
	if (!quiet)
	  error (0, 0, "Ignoring %s", update_dir);
        return R_SKIP_ALL;
      }

    if (!isdir (dir))
    {
	/* if we aren't building dirs, blow it off */
	if (!update_build_dirs)
	    return (R_SKIP_ALL);

	if (noexec)
	{
	    /* ignore the missing dir if -n is specified */
	    error (0, 0, "New directory `%s' -- ignored", dir);
	    return (R_SKIP_ALL);
	}
	else
	{
	    /* otherwise, create the dir and appropriate adm files */
	    make_directory (dir);
	    Create_Admin (dir, update_dir, repository, tag, date);
	}
    }
    /* Do we need to check noexec here? */
    else if (!pipeout)
    {
	char *cvsadmdir;

	/* The directory exists.  Check to see if it has a CVS
	   subdirectory.  */

	cvsadmdir = xmalloc (strlen (dir) + 80);
	strcpy (cvsadmdir, dir);
	strcat (cvsadmdir, "/");
	strcat (cvsadmdir, CVSADM);

	if (!isdir (cvsadmdir))
	{
	    /* We cannot successfully recurse into a directory without a CVS
	       subdirectory.  Generally we will have already printed
	       "? foo".  */
	    free (cvsadmdir);
	    return R_SKIP_ALL;
	}
	free (cvsadmdir);
    }

    /*
     * If we are building dirs and not going to stdout, we make sure there is
     * no static entries file and write the tag file as appropriate
     */
    if (!pipeout)
    {
	if (update_build_dirs)
	{
	    char tmp[PATH_MAX];

	    (void) sprintf (tmp, "%s/%s", dir, CVSADM_ENTSTAT);
	    if (unlink_file (tmp) < 0 && ! existence_error (errno))
		error (1, errno, "cannot remove file %s", tmp);
#ifdef SERVER_SUPPORT
	    if (server_active)
		server_clear_entstat (update_dir, repository);
#endif
	}

	/* keep the CVS/Tag file current with the specified arguments */
	if (aflag || tag || date)
	{
	    WriteTag (dir, tag, date);
#ifdef SERVER_SUPPORT
	    if (server_active)
		server_set_sticky (update_dir, repository, tag, date);
#endif
	}

	/* initialize the ignore list for this directory */
	ignlist = getlist ();
    }

    /* print the warm fuzzy message */
    if (!quiet)
	error (0, 0, "Updating %s", update_dir);

    return (R_PROCESS);
}

/*
 * update_dirleave_proc () is called back by the recursion code upon leaving
 * a directory.  It will prune empty directories if needed and will execute
 * any appropriate update programs.
 */
/* ARGSUSED */
static int
update_dirleave_proc (dir, err, update_dir)
    char *dir;
    int err;
    char *update_dir;
{
    FILE *fp;

    /* run the update_prog if there is one */
    if (err == 0 && !pipeout && !noexec &&
	(fp = fopen (CVSADM_UPROG, "r")) != NULL)
    {
	char *cp;
	char *repository;
	char line[MAXLINELEN];

	repository = Name_Repository ((char *) NULL, update_dir);
	if (fgets (line, sizeof (line), fp) != NULL)
	{
	    if ((cp = strrchr (line, '\n')) != NULL)
		*cp = '\0';
	    run_setup ("%s %s", line, repository);
	    (void) printf ("%s %s: Executing '", program_name, command_name);
	    run_print (stdout);
	    (void) printf ("'\n");
	    (void) run_exec (RUN_TTY, RUN_TTY, RUN_TTY, RUN_NORMAL);
	}
	(void) fclose (fp);
	free (repository);
    }

    /* FIXME: chdir ("..") loses with symlinks.  */
    /* Prune empty dirs on the way out - if necessary */
    (void) chdir ("..");
    if (update_prune_dirs && isemptydir (dir))
    {
	/* I'm not sure the existence_error is actually possible (except
	   in cases where we really should print a message), but since
	   this code used to ignore all errors, I'll play it safe.  */
	if (unlink_file_dir (dir) < 0 && !existence_error (errno))
	    error (0, errno, "cannot remove %s directory", dir);
    }

    return (err);
}

/*
 * Returns 1 if the argument directory is completely empty, other than the
 * existence of the CVS directory entry.  Zero otherwise.
 */
static int
isemptydir (dir)
    char *dir;
{
    DIR *dirp;
    struct dirent *dp;

    if ((dirp = opendir (dir)) == NULL)
    {
	error (0, 0, "cannot open directory %s for empty check", dir);
	return (0);
    }
    while ((dp = readdir (dirp)) != NULL)
    {
	if (strcmp (dp->d_name, ".") != 0 && strcmp (dp->d_name, "..") != 0 &&
	    strcmp (dp->d_name, CVSADM) != 0)
	{
	    (void) closedir (dirp);
	    return (0);
	}
    }
    (void) closedir (dirp);
    return (1);
}

/*
 * scratch the Entries file entry associated with a file
 */
static int
scratch_file (file, repository, entries, update_dir)
    char *file;
    char *repository;
    List *entries;
    char *update_dir;
{
    history_write ('W', update_dir, "", file, repository);
    Scratch_Entry (entries, file);
    (void) unlink_file (file);
    return (0);
}

/*
 * check out a file - essentially returns the result of the fork on "co".
 */
static int
checkout_file (file, repository, entries, rcsnode, vers_ts, update_dir)
    char *file;
    char *repository;
    List *entries;
    RCSNode *rcsnode;
    Vers_TS *vers_ts;
    char *update_dir;
{
    char backup[PATH_MAX];
    int set_time, retval = 0;
    int retcode = 0;
    int status;
    int file_is_dead;

    /* don't screw with backup files if we're going to stdout */
    if (!pipeout)
    {
	(void) sprintf (backup, "%s/%s%s", CVSADM, CVSPREFIX, file);
	if (isfile (file))
	    rename_file (file, backup);
	else
	    (void) unlink_file (backup);
    }

    file_is_dead = RCS_isdead (vers_ts->srcfile, vers_ts->vn_rcs);

    if (!file_is_dead)
    {
	/*
	 * if we are checking out to stdout, print a nice message to
	 * stderr, and add the -p flag to the command */
	if (pipeout)
	{
	    if (!quiet)
	    {
		(void) fprintf (stderr, "\
===================================================================\n");
		if (update_dir[0])
		    (void) fprintf (stderr, "Checking out %s/%s\n",
				    update_dir, file);
		else
		    (void) fprintf (stderr, "Checking out %s\n", file);
		(void) fprintf (stderr, "RCS:  %s\n", vers_ts->srcfile->path);
		(void) fprintf (stderr, "VERS: %s\n", vers_ts->vn_rcs);
		(void) fprintf (stderr, "***************\n");
	    }
	}

	status = RCS_checkout (vers_ts->srcfile->path,
			       pipeout ? NULL : file, vers_ts->vn_tag,
			       vers_ts->options, RUN_TTY, 0, 0);
    }
    if (file_is_dead || status == 0)
    {
	if (!pipeout)
	{
	    Vers_TS *xvers_ts;
	    int resurrecting;

	    resurrecting = 0;

	    if (file_is_dead && joining())
	    {
		if (RCS_getversion (vers_ts->srcfile, join_rev1,
				    date_rev1, 1, 0)
		    || (join_rev2 != NULL && 
			RCS_getversion (vers_ts->srcfile, join_rev2,
					date_rev2, 1, 0)))
		{
		    /* when joining, we need to get dead files checked
		       out.  Try harder.  */
		    /* I think that RCS_FLAGS_FORCE is here only because
		       passing -f to co used to enable checking out
		       a dead revision in the old version of death
		       support which used a hacked RCS instead of using
		       the RCS state.  */
		    retcode = RCS_checkout (vers_ts->srcfile->path, file,
		                            vers_ts->vn_rcs,
		                            vers_ts->options, RUN_TTY,
		                            RCS_FLAGS_FORCE, 0);
		    if (retcode != 0)
		    {
			error (retcode == -1 ? 1 : 0,
			       retcode == -1 ? errno : 0,
			       "could not check out %s", file);
			(void) unlink_file (backup);
			return (retcode);
		    }
		    file_is_dead = 0;
		    resurrecting = 1;
		}
		else
		{
		    /* If the file is dead and does not contain either of
		       the join revisions, then we don't want to check it
		       out. */
		    return 0;
		}
	    }

	    if (cvswrite == TRUE
		&& !file_is_dead
		&& !fileattr_get (file, "_watched"))
		xchmod (file, 1);

	    {
		/* A newly checked out file is never under the spell
		   of "cvs edit".  If we think we were editing it
		   from a previous life, clean up.  Would be better to
		   check for same the working directory instead of
		   same user, but that is hairy.  */

		struct addremove_args args;

		editor_set (file, getcaller (), NULL);

		memset (&args, 0, sizeof args);
		args.remove_temp = 1;
		watch_modify_watchers (file, &args);
	    }

	    /* set the time from the RCS file iff it was unknown before */
	    if (vers_ts->vn_user == NULL ||
		strncmp (vers_ts->ts_rcs, "Initial", 7) == 0)
	    {
		set_time = 1;
	    }
	    else
		set_time = 0;

	    wrap_fromcvs_process_file (file);

	    xvers_ts = Version_TS (repository, options, tag, date, file,
			      force_tag_match, set_time, entries, rcsnode);
	    if (strcmp (xvers_ts->options, "-V4") == 0)
		xvers_ts->options[0] = '\0';

	    (void) time (&last_register_time);

	    if (file_is_dead)
	    {
		if (xvers_ts->vn_user != NULL)
		{
		    if (update_dir[0] == '\0')
			error (0, 0,
			       "warning: %s is not (any longer) pertinent",
			       file);
		    else
			error (0, 0,
			       "warning: %s/%s is not (any longer) pertinent",
			       update_dir, file);
		}
		Scratch_Entry (entries, file);
		if (unlink_file (file) < 0 && ! existence_error (errno))
		{
		    if (update_dir[0] == '\0')
			error (0, errno, "cannot remove %s", file);
		    else
			error (0, errno, "cannot remove %s/%s", update_dir,
			       file);
		}
	    }
	    else
	      Register (entries, file,
			resurrecting ? "0" : xvers_ts->vn_rcs,
			xvers_ts->ts_user, xvers_ts->options,
			xvers_ts->tag, xvers_ts->date,
			(char *)0); /* Clear conflict flag on fresh checkout */

	    /* fix up the vers structure, in case it is used by join */
	    if (join_rev1)
	    {
		if (vers_ts->vn_user != NULL)
		    free (vers_ts->vn_user);
		if (vers_ts->vn_rcs != NULL)
		    free (vers_ts->vn_rcs);
		vers_ts->vn_user = xstrdup (xvers_ts->vn_rcs);
		vers_ts->vn_rcs = xstrdup (xvers_ts->vn_rcs);
	    }

	    /* If this is really Update and not Checkout, recode history */
	    if (strcmp (command_name, "update") == 0)
		history_write ('U', update_dir, xvers_ts->vn_rcs, file,
			       repository);

	    freevers_ts (&xvers_ts);

	    if (!really_quiet && !file_is_dead)
	    {
		write_letter (file, 'U', update_dir);
	    }
	}
    }
    else
    {
	int old_errno = errno;		/* save errno value over the rename */

	if (!pipeout && isfile (backup))
	    rename_file (backup, file);

	error (retcode == -1 ? 1 : 0, retcode == -1 ? old_errno : 0,
	       "could not check out %s", file);

	retval = retcode;
    }

    if (!pipeout)
	(void) unlink_file (backup);

    return (retval);
}

#ifdef SERVER_SUPPORT
/* Patch a file.  Runs rcsdiff.  This is only done when running as the
 * server.  The hope is that the diff will be smaller than the file
 * itself.
 */
static int
patch_file (file, repository, entries, rcsnode, vers_ts, update_dir,
	    docheckout, file_info, checksum)
    char *file;
    char *repository;
    List *entries;
    RCSNode *rcsnode;
    Vers_TS *vers_ts;
    char *update_dir;
    int *docheckout;
    struct stat *file_info;
    unsigned char *checksum;
{
    char backup[PATH_MAX];
    char file1[PATH_MAX];
    char file2[PATH_MAX];
    int retval = 0;
    int retcode = 0;
    int fail;
    FILE *e;

    *docheckout = 0;

    if (pipeout || joining ())
    {
	*docheckout = 1;
	return 0;
    }

    (void) sprintf (backup, "%s/%s%s", CVSADM, CVSPREFIX, file);
    if (isfile (file))
        rename_file (file, backup);
    else
        (void) unlink_file (backup);
    
    (void) sprintf (file1, "%s/%s%s-1", CVSADM, CVSPREFIX, file);
    (void) sprintf (file2, "%s/%s%s-2", CVSADM, CVSPREFIX, file);

    fail = 0;

    /* We need to check out both revisions first, to see if either one
       has a trailing newline.  Because of this, we don't use rcsdiff,
       but just use diff.  */
    if (noexec)
	retcode = 0;
    else
	retcode = RCS_checkout (vers_ts->srcfile->path, NULL,
	                        vers_ts->vn_user,
	                        vers_ts->options, file1, 0, 0);
    if (retcode != 0)
        fail = 1;
    else
    {
        e = fopen (file1, "r");
	if (e == NULL)
	    fail = 1;
	else
	{
	    if (fseek (e, (long) -1, SEEK_END) == 0
		&& getc (e) != '\n')
	    {
	        fail = 1;
	    }
	    fclose (e);
	}
    }

    if (! fail)
    {
        /* Check it out into file, and then move to file2, so that we
           can get the right modes into *FILE_INFO.  We can't check it
           out directly into file2 because co doesn't understand how
           to do that.  */
	retcode = RCS_checkout (vers_ts->srcfile->path, file,
	                        vers_ts->vn_rcs,
	                        vers_ts->options, RUN_TTY, 0, 0);
	if (retcode != 0)
	    fail = 1;
	else
	{
	    if (!isreadable (file))
	    {
	        /* File is dead.  */
	        fail = 1;
	    }
	    else
	    {
	        rename_file (file, file2);
		if (cvswrite == TRUE
		    && !fileattr_get (file, "_watched"))
		    xchmod (file2, 1);
		e = fopen (file2, "r");
		if (e == NULL)
		    fail = 1;
		else
		{
		    struct MD5Context context;
		    int nl;
		    unsigned char buf[8192];
		    unsigned len;

		    nl = 0;

		    /* Compute the MD5 checksum and make sure there is
                       a trailing newline.  */
		    MD5Init (&context);
		    while ((len = fread (buf, 1, sizeof buf, e)) != 0)
		    {
			nl = buf[len - 1] == '\n';
		        MD5Update (&context, buf, len);
		    }
		    MD5Final (checksum, &context);

		    if (ferror (e) || ! nl)
		    {
		        fail = 1;
		    }

		    fclose (e);
		}
	    }
	}
    }	  

    retcode = 0;
    if (! fail)
    {
	/* FIXME: This whole thing with diff/patch is rather more
	   convoluted than necessary (lots of forks and execs, need to
	   worry about versions of diff and patch, etc.).  Also, we
	   send context lines which aren't needed (in the rare case in
	   which the diff doesn't apply, the checksum would catches it).
	   Solution perhaps is to librarify the RCS routines which apply
	   deltas or something equivalent.  */
	/* This is -c, not -u, because we have no way of knowing which
	   DIFF is in use.  */
	run_setup ("%s -c %s %s", DIFF, file1, file2);

	/* A retcode of 0 means no differences.  1 means some differences.  */
	if ((retcode = run_exec (RUN_TTY, file, RUN_TTY, RUN_NORMAL)) != 0
	    && retcode != 1)
	{
	    fail = 1;
	}
	else
	{
#define BINARY "Binary"
	    char buf[sizeof BINARY];
	    unsigned int c;

	    /* Check the diff output to make sure patch will be handle it.  */
	    e = fopen (file, "r");
	    if (e == NULL)
		error (1, errno, "could not open diff output file %s", file);
	    c = fread (buf, 1, sizeof BINARY - 1, e);
	    buf[c] = '\0';
	    if (strcmp (buf, BINARY) == 0)
	    {
		/* These are binary files.  We could use diff -a, but
		   patch can't handle that.  */
		fail = 1;
	    }
	    fclose (e);
	}
    }

    if (! fail)
    {
        Vers_TS *xvers_ts;

        /* This stuff is just copied blindly from checkout_file.  I
	   don't really know what it does.  */
        xvers_ts = Version_TS (repository, options, tag, date, file,
			       force_tag_match, 0, entries, rcsnode);
	if (strcmp (xvers_ts->options, "-V4") == 0)
	    xvers_ts->options[0] = '\0';

	Register (entries, file, xvers_ts->vn_rcs,
		  xvers_ts->ts_user, xvers_ts->options,
		  xvers_ts->tag, xvers_ts->date, NULL);

	if (stat (file2, file_info) < 0)
	    error (1, errno, "could not stat %s", file2);

	/* If this is really Update and not Checkout, recode history */
	if (strcmp (command_name, "update") == 0)
	    history_write ('P', update_dir, xvers_ts->vn_rcs, file,
			   repository);

	freevers_ts (&xvers_ts);

	if (!really_quiet)
	{
	    write_letter (file, 'P', update_dir);
	}
    }
    else
    {
	int old_errno = errno;		/* save errno value over the rename */

	if (isfile (backup))
	    rename_file (backup, file);

	if (retcode != 0 && retcode != 1)
	    error (retcode == -1 ? 1 : 0, retcode == -1 ? old_errno : 0,
		   "could not diff %s", file);

	*docheckout = 1;
	retval = retcode;
    }

    (void) unlink_file (backup);
    (void) unlink_file (file1);
    (void) unlink_file (file2);

    return (retval);
}
#endif

/*
 * Several of the types we process only print a bit of information consisting
 * of a single letter and the name.
 */
static int
write_letter (file, letter, update_dir)
    char *file;
    int letter;
    char *update_dir;
{
    if (!really_quiet)
    {
	char buf[2];
	buf[0] = letter;
	buf[1] = ' ';
	cvs_output (buf, 2);
	if (update_dir[0])
	{
	    cvs_output (update_dir, 0);
	    cvs_output ("/", 1);
	}
	cvs_output (file, 0);
	cvs_output ("\n", 1);
    }
    return (0);
}

/*
 * Do all the magic associated with a file which needs to be merged
 */
static int
merge_file (file, repository, entries, vers, update_dir)
    char *file;
    char *repository;
    List *entries;
    Vers_TS *vers;
    char *update_dir;
{
    char user[PATH_MAX];
    char backup[PATH_MAX];
    int status;
    int retcode = 0;

    /*
     * The users currently modified file is moved to a backup file name
     * ".#filename.version", so that it will stay around for a few days
     * before being automatically removed by some cron daemon.  The "version"
     * is the version of the file that the user was most up-to-date with
     * before the merge.
     */
    (void) sprintf (backup, "%s%s.%s", BAKPREFIX, file, vers->vn_user);
    if (update_dir[0])
	(void) sprintf (user, "%s/%s", update_dir, file);
    else
	(void) strcpy (user, file);

    (void) unlink_file (backup);
    copy_file (file, backup);
    xchmod (file, 1);

    status = RCS_merge(vers->srcfile->path, 
		       vers->options, vers->vn_user, vers->vn_rcs);
    if (status != 0 && status != 1)
    {
	error (0, status == -1 ? errno : 0,
	       "could not merge revision %s of %s", vers->vn_user, user);
	error (status == -1 ? 1 : 0, 0, "restoring %s from backup file %s",
	       user, backup);
	rename_file (backup, file);
	return (1);
    }

    if (strcmp (vers->options, "-V4") == 0)
	vers->options[0] = '\0';
    (void) time (&last_register_time);
    {
	char *cp = 0;

	if (status)
	    cp = time_stamp (file);
	Register (entries, file, vers->vn_rcs, vers->ts_rcs, vers->options,
		  vers->tag, vers->date, cp);
	if (cp)
	    free (cp);
    }

    /* fix up the vers structure, in case it is used by join */
    if (join_rev1)
    {
	if (vers->vn_user != NULL)
	    free (vers->vn_user);
	vers->vn_user = xstrdup (vers->vn_rcs);
    }

#ifdef SERVER_SUPPORT
    /* Send the new contents of the file before the message.  If we
       wanted to be totally correct, we would have the client write
       the message only after the file has safely been written.  */
    if (server_active)
    {
        server_copy_file (file, update_dir, repository, backup);
	server_updated (file, update_dir, repository, SERVER_MERGED,
			(struct stat *) NULL, (unsigned char *) NULL);
    }
#endif

    if (!noexec && !xcmp (backup, file))
    {
	printf ("%s already contains the differences between %s and %s\n",
		user, vers->vn_user, vers->vn_rcs);
	history_write ('G', update_dir, vers->vn_rcs, file, repository);
	return (0);
    }

    if (status == 1)
    {
	if (!noexec)
	    error (0, 0, "conflicts found in %s", user);

	write_letter (file, 'C', update_dir);

	history_write ('C', update_dir, vers->vn_rcs, file, repository);

    }
    else if (retcode == -1)
    {
	error (1, errno, "fork failed while examining update of %s", user);
    }
    else
    {
	write_letter (file, 'M', update_dir);
	history_write ('G', update_dir, vers->vn_rcs, file, repository);
    }
    return (0);
}

/*
 * Do all the magic associated with a file which needs to be joined
 * (-j option)
 */
static void
#ifdef SERVER_SUPPORT
join_file (file, rcsnode, vers, update_dir, entries, repository)
    char *repository;
#else
join_file (file, rcsnode, vers, update_dir, entries)
#endif
    char *file;
    RCSNode *rcsnode;
    Vers_TS *vers;
    char *update_dir;
    List *entries;
{
    char user[PATH_MAX];
    char backup[PATH_MAX];
    char *options;
    int status;

    char *rev1;
    char *rev2;
    char *jrev1;
    char *jrev2;
    char *jdate1;
    char *jdate2;

    jrev1 = join_rev1;
    jrev2 = join_rev2;
    jdate1 = date_rev1;
    jdate2 = date_rev2;

    if (wrap_merge_is_copy (file))
    {
	/* FIXME: Should be including update_dir in message.  */
	error (0, 0,
	       "Cannot merge %s because it is a merge-by-copy file.", file);
	return;
    }

    /* determine if we need to do anything at all */
    if (vers->srcfile == NULL ||
	vers->srcfile->path == NULL)
    {
	return;
    }

    /* in all cases, use two revs. */

    /* if only one rev is specified, it becomes the second rev */
    if (jrev2 == NULL)
    {
	jrev2 = jrev1;
	jrev1 = NULL;
	jdate2 = jdate1;
	jdate1 = NULL;
    }

    /* The file in the working directory doesn't exist in CVS/Entries.
       FIXME: Shouldn't this case result in additional processing (if
       the file was added going from rev1 to rev2, then do the equivalent
       of a "cvs add")?  (yes; easier said than done.. :-) */
    if (vers->vn_user == NULL)
    {
	/* No merge possible YET. */
	if (jdate2 != NULL)
	    error (0, 0,
		   "file %s is present in revision %s as of %s",
		   file, jrev2, jdate2);
	else
	    error (0, 0,
		   "file %s is present in revision %s",
		   file, jrev2);
	return;
    }

    /* Fix for bug CVS/193:
     * Used to dump core if the file had been removed on the current branch.
     */
    if (strcmp(vers->vn_user, "0") == 0)
    {
        error(0, 0,
              "file %s has been deleted",
              file);
        return;
    }

    /* convert the second rev spec, walking branches and dates. */

    rev2 = RCS_getversion (vers->srcfile, jrev2, jdate2, 1, 0);
    if (rev2 == NULL)
    {
	if (!quiet)
	{
	    if (jdate2 != NULL)
		error (0, 0,
		       "cannot find revision %s as of %s in file %s",
		       jrev2, jdate2, file);
	    else
		error (0, 0,
		       "cannot find revision %s in file %s",
		       jrev2, file);
	}
	return;
    }

    /* skip joining identical revs */
    if (strcmp (rev2, vers->vn_user) == 0)
    {
	/* No merge necessary.  */
	free (rev2);
	return;
    }

    if (jrev1 == NULL)
    {
	char *tst;
	/* if the first rev is missing, then it is implied to be the
	   greatest common ancestor of both the join rev, and the
	   checked out rev. */
	
	/* FIXME: What is this check for '!' about?  If it is legal to
	   have '!' in the first character of vn_user, it isn't
	   documented at struct vers_ts in cvs.h.  */
	tst = vers->vn_user;
	if (*tst == '!')
	{
	    /* file was dead.  merge anyway and pretend it's been
	       added. */
	    ++tst;
	    Register (entries, file, "0", vers->ts_user, vers->options,
		      vers->tag, (char *) 0, (char *) 0);
	}
	rev1 = gca (tst, rev2);
	if (rev1 == NULL)
	{
	    /* this should not be possible */
	    error (0, 0, "bad gca");
	    abort();
	}

	tst = RCS_gettag (vers->srcfile, rev2, 1, 0);
	if (tst == NULL)
	{
	    /* this should not be possible. */
	    error (0, 0, "cannot find gca");
	    abort();
	}

	free (tst);

	/* these two cases are noops */
	if (strcmp (rev1, rev2) == 0)
	{
	    free (rev1);
	    free (rev2);
	    return;
	}
    }
    else
    {
	/* otherwise, convert the first rev spec, walking branches and
	   dates.  */

	rev1 = RCS_getversion (vers->srcfile, jrev1, jdate1, 1, 0);
	if (rev1 == NULL)
	{
	  if (!quiet) {
	    if (jdate1 != NULL)
		error (0, 0,
		       "cannot find revision %s as of %s in file %s",
		       jrev1, jdate1, file);
	    else
		error (0, 0,
		       "cannot find revision %s in file %s",
		       jrev1, file);
	  }
	  return;
	}
    }

    /* do the join */

#if 0
    dome {
	/* special handling when two revisions are specified */
	if (join_rev1 && join_rev2)
	{
	    rev = RCS_getversion (vers->srcfile, join_rev2, date_rev2, 1, 0);
	    if (rev == NULL)
	    {
		if (!quiet && date_rev2 == NULL)
		    error (0, 0,
			   "cannot find revision %s in file %s", join_rev2, file);
		return;
	    }
	    
	    baserev = RCS_getversion (vers->srcfile, join_rev1, date_rev1, 1, 0);
	    if (baserev == NULL)
	    {
		if (!quiet && date_rev1 == NULL)
		    error (0, 0,
			   "cannot find revision %s in file %s", join_rev1, file);
		free (rev);
		return;
	    }
	    
	    /*
	     * nothing to do if:
	     *	second revision matches our BASE revision (vn_user) &&
	     *	both revisions are on the same branch
	     */
	    if (strcmp (vers->vn_user, rev) == 0 &&
		numdots (baserev) == numdots (rev))
	    {
		/* might be the same branch.  take a real look */
		char *dot = strrchr (baserev, '.');
		int len = (dot - baserev) + 1;
		
		if (strncmp (baserev, rev, len) == 0)
		    return;
	    }
	}
	else
	{
	    rev = RCS_getversion (vers->srcfile, join_rev1, date_rev1, 1, 0);
	    if (rev == NULL)
		return;
	    if (strcmp (rev, vers->vn_user) == 0) /* no merge necessary */
	    {
		free (rev);
		return;
	    }
	    
	    baserev = RCS_whatbranch (file, join_rev1, rcsnode);
	    if (baserev)
	    {
		char *cp;
		
		/* we get a branch -- turn it into a revision, or NULL if trunk */
		if ((cp = strrchr (baserev, '.')) == NULL)
		{
		    free (baserev);
		    baserev = (char *) NULL;
		}
		else
		    *cp = '\0';
	    }
	}
	if (baserev && strcmp (baserev, rev) == 0)
	{
	    /* they match -> nothing to do */
	    free (rev);
	    free (baserev);
	    return;
	}
    }
#endif

    /* OK, so we have two revisions; continue on */

#ifdef SERVER_SUPPORT
    if (server_active && !isreadable (file))
    {
	int retcode;
	/* The file is up to date.  Need to check out the current contents.  */
	retcode = RCS_checkout (vers->srcfile->path, "", vers->vn_user, NULL,
	                        RUN_TTY, 0, 0);
	if (retcode != 0)
	    error (1, retcode == -1 ? errno : 0,
		   "failed to check out %s file", file);
    }
#endif
    
    /*
     * The users currently modified file is moved to a backup file name
     * ".#filename.version", so that it will stay around for a few days
     * before being automatically removed by some cron daemon.  The "version"
     * is the version of the file that the user was most up-to-date with
     * before the merge.
     */
    (void) sprintf (backup, "%s%s.%s", BAKPREFIX, file, vers->vn_user);
    if (update_dir[0])
	(void) sprintf (user, "%s/%s", update_dir, file);
    else
	(void) strcpy (user, file);

    (void) unlink_file (backup);
    copy_file (file, backup);
    xchmod (file, 1);

    options = vers->options;
#ifdef HAVE_RCS5
#if 0
    if (*options == '\0')
	options = "-kk";		/* to ignore keyword expansions */
#endif
#endif

    status = RCS_merge (vers->srcfile->path, options, rev1, rev2);
    if (status != 0 && status != 1)
    {
	error (0, status == -1 ? errno : 0,
	       "could not merge revision %s of %s", rev2, user);
	error (status == -1 ? 1 : 0, 0, "restoring %s from backup file %s",
	       user, backup);
	rename_file (backup, file);
    }
    free (rev1);
    free (rev2);

#ifdef SERVER_SUPPORT
    /*
     * If we're in server mode, then we need to re-register the file
     * even if there were no conflicts (status == 0).
     * This tells server_updated() to send the modified file back to
     * the client.
     */
    if (status == 1 || (status == 0 && server_active))
#else
    if (status == 1)
#endif
    {
	char *cp = 0;

	if (status)
	    cp = time_stamp (file);
	Register (entries, file, vers->vn_rcs, vers->ts_rcs, vers->options,
		  vers->tag, vers->date, cp);
	if (cp)
	    free(cp);
    }

#ifdef SERVER_SUPPORT
    if (server_active)
    {
	server_copy_file (file, update_dir, repository, backup);
	server_updated (file, update_dir, repository, SERVER_MERGED,
			(struct stat *) NULL, (unsigned char *) NULL);
    }
#endif
}

int
joining ()
{
    return (join_rev1 != NULL);
}
