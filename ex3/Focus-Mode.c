#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

/*
Define the signals of every functionality
*/
#define SIG_EMAIL SIGUSR1  // Email notification
#define SIG_REMIND SIGUSR2 // remind to pick delivery
#define SIG_DB SIGALRM     // doorbell ringing

// function that implement the ouput for task 1 = email notification
void handle_email()
{
    printf(" - Email notification is waiting.\n");
    printf("[Outcome:] The TA announced: Everyone get 100 on the exercise!\n");
}

// function that implement the ouput for task 2 = remind to pick delivery
void handle_reminder()
{
    printf(" - You have a reminder to pick up your delivery.\n");
    printf("[Outcome:] You picked it up just in time.\n");
}

// function that implement the ouput for task 3 = Doorbell ringing
void handle_doorbell()
{
    printf(" - The doorbell is ringing.\n");
    printf("[Outcome:] Food delivery is here.\n");
}

void menu()
{
    printf("Simulate a distraction:\n");
    printf("  1 = Email notification\n");
    printf("  2 = Reminder to pick up delivery\n");
    printf("  3 = Doorbell Ringing\n");
    printf("  q = Quit\n\n");
}

int check(sigset_t set, int signum, struct sigaction sa) {
    // if the signal is not in the set of pending, we will return 0 and continue
    if (!sigismember(&set, signum)) return 0;
    
    // Make sure the handler isn't SIG_IGN or SIG_DFL
    if (sa.sa_handler != SIG_IGN && sa.sa_handler != SIG_DFL) {
        // Call the handler function
        sa.sa_handler(signum);
        return 1;
    }
    // Return 0 if handler is SIG_IGN or SIG_DFL
    return 0; 
}


void runFocusMode(int numOfRounds, int duration)
{
    struct sigaction sa1;
    struct sigaction sa2;
    struct sigaction sa3;

    // insert details to siaction 1:
    sa1.sa_handler = handle_email;
    sigemptyset(&sa1.sa_mask);
    sa1.sa_flags = 0;
    sigaction(SIG_EMAIL, &sa1, NULL);
    // insert details to siaction 2:
    sa2.sa_handler = handle_reminder;
    sigemptyset(&sa2.sa_mask);
    sa2.sa_flags = 0;
    sigaction(SIG_REMIND, &sa2, NULL);
    // insert details to siaction 3:
    sa3.sa_handler = handle_doorbell;
    sigemptyset(&sa3.sa_mask);
    sa3.sa_flags = 0;
    sigaction(SIG_DB, &sa3, NULL);

    // initialize set of signals
    sigset_t actions;

   
    

    for (int i = 0; i < numOfRounds; i++)
    {
         // inialize set of pending signals
        sigset_t pending;
        sigemptyset(&pending);
        // clean the sets
        sigemptyset(&actions);
        
        // add the signals of the actions to the set
        sigaddset(&actions, SIG_EMAIL);
        sigaddset(&actions, SIG_REMIND);
        sigaddset(&actions, SIG_DB);
        // block the signals that handel the actions untill released
        sigprocmask(SIG_BLOCK, &actions, NULL); 

        printf("Entering Focus Mode. All distractions are blocked.\n");
        printf("══════════════════════════════════════════════\n");
        printf("                Focus Round %d                \n", i + 1);
        printf("──────────────────────────────────────────────\n");

        for (int j = 0; j < duration; j++)
        {
            char choice;
            menu();
            scanf(" %c", &choice);
            // check if the user wants to quit the mode
            if (choice == 'q')
            {
                break;
            }

            switch (choice)
            {
            case '1':
                raise(SIG_EMAIL);
                break;
            case '2':
                raise(SIG_REMIND);
                break;
            case '3':
                raise(SIG_DB);
                break;
            default:
                break;
            }
        }
        printf("──────────────────────────────────────────────\n");
        printf("        Checking pending distractions...      \n");
        printf("──────────────────────────────────────────────\n");

        if (sigpending(&pending) == -1) {
            perror("sigpending");
            exit(EXIT_FAILURE);
        }

        // implement the tasks
        int counter = 0;
        counter += check(pending, SIG_EMAIL, sa1);
        counter += check(pending, SIG_REMIND, sa2);
        counter += check(pending, SIG_DB, sa3);

        if (counter == 0) {
            printf("No distractions reached you this round.\n");
        }

        // Unblock signals to handle them
        sigprocmask(SIG_UNBLOCK, &pending, NULL);

        printf("──────────────────────────────────────────────\n");
        printf("             Back to Focus Mode.              \n");
        printf("══════════════════════════════════════════════\n");
    }
    printf("Focus Mode complete. All distractions are now unblocked.\n");
}