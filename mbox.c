#include "common.h"
#include "mbox.h"
#include "sync.h"
#include "util.h"

#define BUFFER_LENGTH (MAX_MBOX_LENGTH + 1)

typedef struct
{
  // Fill this in
  char msg[MAX_MESSAGE_LENGTH];
} Message;

typedef struct
{
  char name[MBOX_NAME_LENGTH];
  // Fill this in
  int usage_count;
  Message messages[BUFFER_LENGTH];
  int start, end;
  semaphore_t full_count;
  semaphore_t empty_count;
  lock_t lock;
} MessageBox;


static MessageBox MessageBoxen[MAX_MBOXEN];

/* Perform any system-startup
 * initialization for the message
 * boxes.
 */
void init_mbox(void)
{
  int i;
  (void) MessageBoxen;
  // Fill this in
  for (i = 0; i < MAX_MBOXEN; i++) {
    MessageBoxen[i].usage_count = 0; 
    MessageBoxen[i].start = 0; 
    MessageBoxen[i].end = 0;
    semaphore_init(&MessageBoxen[i].full_count, 0);
    semaphore_init(&MessageBoxen[i].empty_count, MAX_MBOX_LENGTH);
    lock_init(&MessageBoxen[i].lock); 
  }
}

/* Opens the mailbox named 'name', or
 * creates a new message box if it
 * doesn't already exist.
 * A message box is a bounded buffer
 * which holds up to MAX_MBOX_LENGTH items.
 * If it fails because the message
 * box table is full, it will return -1.
 * Otherwise, it returns a message box
 * id.
 */
mbox_t do_mbox_open(const char *name)
{
    int i;
    (void)name;
  
    for (i = 0; i < MAX_MBOXEN; i++) {
        if (same_string(MessageBoxen[i].name, name)) { 
            MessageBoxen[i].usage_count++;
            return i; 
        }
    }
  
    for (i = 0; i < MAX_MBOXEN; i++) {
        if (MessageBoxen[i].usage_count == 0) {
            int n = strlen((char*)name);
            bcopy((char*)name, MessageBoxen[i].name, (n*sizeof(char)));
            MessageBoxen[i].usage_count++;
            return i;
        }
    }

    return -1;
}

/* Closes a message box
 */
void do_mbox_close(mbox_t mbox)
{
    (void)mbox;
    MessageBoxen[mbox].usage_count--;
    if (MessageBoxen[mbox].usage_count == 0) {
        semaphore_init(&MessageBoxen[mbox].full_count, 0);
        semaphore_init(&MessageBoxen[mbox].empty_count, MAX_MBOX_LENGTH);
    }
}

/* Determine if the given
 * message box is full.
 * Equivalently, determine
 * if sending to this mbox
 * would cause a process
 * to block.
 */
int do_mbox_is_full(mbox_t mbox)
{
  (void)mbox;
  // fill this in
  return ((MessageBoxen[mbox].end + 1) % BUFFER_LENGTH) == MessageBoxen[mbox].start;
}

/* Enqueues a message onto
 * a message box.  If the
 * message box is full, the
 * process will block until
 * it can add the item.
 * You may assume that the
 * message box ID has been
 * properly opened before this
 * call.
 * The message is 'nbytes' bytes
 * starting at 'msg'
 */
void do_mbox_send(mbox_t mbox, void *msg, int nbytes)
{
  (void)mbox;
  (void)msg;
  (void)nbytes;
  // fill this in
  MessageBox mb = MessageBoxen[mbox];
  semaphore_down(&mb.empty_count);
  lock_acquire(&mb.lock);
  bcopy(msg, mb.messages[mb.end].msg, nbytes);
  mb.end = (mb.end + 1) % BUFFER_LENGTH;
  lock_release(&mb.lock);
  semaphore_up(&mb.full_count);
}

/* Receives a message from the
 * specified message box.  If
 * empty, the process will block
 * until it can remove an item.
 * You may assume that the
 * message box has been properly
 * opened before this call.
 * The message is copied into
 * 'msg'.  No more than
 * 'nbytes' bytes will be copied
 * into this buffer; longer
 * messages will be truncated.
 */
void do_mbox_recv(mbox_t mbox, void *msg, int nbytes)
{
  (void)mbox;
  (void)msg;
  (void)nbytes;
  // fill this in
  MessageBox mb = MessageBoxen[mbox];
  //asm("xchg %bx, %bx");
  semaphore_down(&mb.full_count);
  lock_acquire(&mb.lock);
  bcopy(mb.messages[mb.start].msg, msg, nbytes);
  mb.start = (mb.start + 1) % BUFFER_LENGTH;
  lock_release(&mb.lock);
  semaphore_up(&mb.empty_count);
}

/* Returns the number of processes that have
 *  * opened but not closed this mailbox.
 *   */
unsigned int do_mbox_usage_count(mbox_t mbox)
{
  (void)mbox;
  return MessageBoxen[mbox].usage_count;
}

