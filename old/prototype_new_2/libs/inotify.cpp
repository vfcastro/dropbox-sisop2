#include "event_queue.h"
#include "inotify.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/select.h>
#include <signal.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include "SendAction.cpp"

extern int keep_running;
static int watched_items;

int keep_running;

/* This program will take as arguments one or more directory
   or file names, and monitor them, printing event notifications
   to the console. It will automatically terminate if all watched
   items are deleted or unmounted. Use ctrl-C or kill to
   terminate otherwise.
*/

/* Signal handler that simply resets a flag to cause termination */
void signal_handler (int signum)
{
    keep_running = 0;
}

int inotifyFunc(char* dirName){
    /* This is the file descriptor for the inotify watch */
    int inotify_fd;

    keep_running = 1;

    /* Set a ctrl-c signal handler */
    if (signal (SIGINT, signal_handler) == SIG_IGN){
        /* Reset to SIG_IGN (ignore) if that was the prior state */
        signal (SIGINT, SIG_IGN);
    }

    /* First we open the inotify dev entry */
    inotify_fd = open_inotify_fd ();
    if (inotify_fd > 0){

        /* We will need a place to enqueue inotify events,
           this is needed because if you do not read events
           fast enough, you will miss them. This queue is
           probably too small if you are monitoring something
           like a directory with a lot of files and the directory
           is deleted.
         */
        queue_t q;
        q = queue_create ();

        /* This is the watch descriptor returned for each item we are
           watching. A real application might keep these for some use
           in the application. This sample only makes sure that none of
           the watch descriptors is less than 0.
         */
        int wd;

        int index;
        wd = 0;
        printf("\n");

        wd = watch_dir(inotify_fd,dirName,IN_CLOSE_WRITE|IN_DELETE|IN_MOVED_TO|IN_MOVED_FROM);

        if (wd > 0){
           /* Wait for events and process them until a
           termination condition is detected
          */
            process_inotify_events (q, inotify_fd);
        }
        printf ("\nTerminating\n");

        /* Finish up by closing the fd, destroying the queue,
           and returning a proper code
         */
        close_inotify_fd (inotify_fd);
        queue_destroy (q);
    }
}

/* Create an inotify instance and open a file descriptor
   to access it */
int open_inotify_fd ()
{
    int fd;

    watched_items = 0;
    fd = inotify_init ();

    if(fd < 0){
        perror ("inotify_init () = ");
    }

    return fd;
}


/* Close the open file descriptor that was opened with inotify_init() */
int close_inotify_fd (int fd)
{
    int r;

    if((r = close (fd)) < 0){
        perror ("close (fd) = ");
    }

    watched_items = 0;
    return r;
}

/* This method does the work of determining what happened,
   then allows us to act appropriately
*/
void handle_event (queue_entry_t event)
{
    /* If the event was associated with a filename, we will store it here */
    char *cur_event_filename = NULL;
    char *cur_event_file_or_dir = NULL;
    /* This is the watch descriptor the event occurred on */
    int cur_event_wd = event->inot_ev.wd;
    int cur_event_cookie = event->inot_ev.cookie;
    unsigned long flags;

    if (event->inot_ev.len){
        cur_event_filename = event->inot_ev.name;
    }
    if( event->inot_ev.mask & IN_ISDIR ){
        cur_event_file_or_dir = (char*)malloc(sizeof(char)*4);
        strcpy(cur_event_file_or_dir, "Dir");
    }
    else{
        cur_event_file_or_dir = (char*)malloc(sizeof(char)*4);
        strcpy(cur_event_file_or_dir, "File");
    }

    flags = event->inot_ev.mask & ~(IN_ALL_EVENTS | IN_UNMOUNT | IN_Q_OVERFLOW | IN_IGNORED );

    /* Perform event dependent handler routines */
    /* The mask is the magic that tells us what file operation occurred */
    SendAction a;
    switch (event->inot_ev.mask & (IN_ALL_EVENTS | IN_UNMOUNT | IN_Q_OVERFLOW | IN_IGNORED)){
    /* File was accessed */
    case IN_ACCESS:
    /* File changed attributes */
    case IN_ATTRIB:
    /* File open read-only was closed */
    case IN_CLOSE_NOWRITE:
    /* File was opened */
    case IN_OPEN:
    /* Working dir was deleted */
    case IN_DELETE_SELF:
    /* Subdir or file was created */
    case IN_CREATE:
    /* Watched entry was moved */
    case IN_MOVE_SELF:
    /* Backing FS was unmounted */
    case IN_UNMOUNT:
        break;

      /* File was modified */
    case IN_MODIFY:
      //Normaly is not necessary because is always called after close_write or moved_to
        break;

      /* File open for writing was closed */
    case IN_CLOSE_WRITE:
        a.update(cur_event_filename, true);
        break;

      /* File was moved from X */
    case IN_MOVED_FROM:
        a.deleteF(cur_event_filename,true);
        break;

      /* File was moved to X */
    case IN_MOVED_TO:
        a.update(cur_event_filename,true);
        break;

        /* Subdir or file was deleted */
    case IN_DELETE:
        a.deleteF(cur_event_filename,true);
        break;

      /* Too many FS events were received without reading them
         some event notifications were potentially lost.  */
    case IN_Q_OVERFLOW:
        printf ("Warning: AN OVERFLOW EVENT OCCURRED: \n");
        break;

      /* Watch was removed explicitly by inotify_rm_watch or automatically
         because file was deleted, or file system was unmounted.  */
    case IN_IGNORED:
        watched_items--;
        printf ("IGNORED: WD #%d\n", cur_event_wd);
        printf("Watching = %d items\n",watched_items);
        break;

      /* Some unknown message received */
    default:
        printf ("UNKNOWN EVENT \"%X\" OCCURRED for file \"%s\" on WD #%i\n",
        event->inot_ev.mask, cur_event_filename, cur_event_wd);
        break;
    }

    /* If any flags were set other than IN_ISDIR, report the flags */
    if (flags & (~IN_ISDIR)){
        flags = event->inot_ev.mask;
        printf ("Flags=%lX\n", flags);
    }
}

void handle_events (queue_t q)
{
  queue_entry_t event;
  while (!queue_empty (q))
    {
      event = queue_dequeue (q);
      handle_event (event);
      free (event);
    }
}

int read_events (queue_t q, int fd)
{
    char buffer[16384];
    size_t buffer_i;
    struct inotify_event *pevent;
    queue_entry_t event;
    ssize_t r;
    size_t event_size, q_event_size;
    int count = 0;

    r = read (fd, buffer, 16384);
    if (r <= 0)
        return r;
    buffer_i = 0;
    while (buffer_i < r){
        /* Parse events and queue them. */
        pevent = (struct inotify_event *) &buffer[buffer_i];
        event_size =  offsetof (struct inotify_event, name) + pevent->len;
        q_event_size = offsetof (struct queue_entry, inot_ev.name) + pevent->len;
        event = (queue_entry_t) malloc (q_event_size);
        memmove (&(event->inot_ev), pevent, event_size);
        queue_enqueue (event, q);
        buffer_i += event_size;
        count++;
    }

    printf ("\n%d events queued\n", count);
    return count;
}

int event_check (int fd){
    fd_set rfds;
    FD_ZERO (&rfds);
    FD_SET (fd, &rfds);
    /* Wait until an event happens or we get interrupted
    by a signal that we catch */
    return select (FD_SETSIZE, &rfds, NULL, NULL, NULL);
}

int process_inotify_events (queue_t q, int fd)
{
    while (keep_running && (watched_items > 0))
    {
        if (event_check (fd) > 0){
            int r;
            r = read_events (q, fd);
            if (r < 0){
                break;
            }
            else{
                handle_events (q);
            }
        }
    }
    return 0;
}

int watch_dir (int fd, const char *dirname, unsigned long mask)
{
    int wd;
    wd = inotify_add_watch (fd, dirname, mask);
    if (wd < 0){
        printf("Cannot add watch for \"%s\" with event mask %lX", dirname, mask);
        fflush(stdout);
        perror(" ");
    }

    else{
        watched_items++;
        printf ("Watching %s WD=%d\n", dirname, wd);
        printf ("Watching = %d items\n", watched_items);
    }

    return wd;
}

int ignore_wd (int fd, int wd)
{
    int r;
    r = inotify_rm_watch (fd, wd);
    if (r < 0){
        perror ("inotify_rm_watch(fd, wd) = ");
    }
    else{
      watched_items--;
    }

    return r;
}
