#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

/*
Define the signals of every functionality
*/
#define SIG_EMAIL SIGUSR1  // Email notification
#define SIG_REMIND SIGUSR2 // Reminder to pick up delivery
#define SIG_DB SIGALRM     // Doorbell ringing

// function that implement the output for task 1 = email notification
void handle_email()
{
    printf(" - Email notification is waiting.\n");
    printf("[Outcome:] The TA announced: Everyone get 100 on the exercise!\n");
}

// function that implement the output for task 2 = remind to pick delivery
void handle_reminder()
{
    printf(" - You have a reminder to pick up your delivery.\n");
    printf("[Outcome:] You picked it up just in time.\n");
}

// function that implement the output for task 3 = Doorbell ringing
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
    printf("  q = Quit\n");
}

int check(sigset_t set, int signum, struct sigaction sa) {
    if (!sigismember(&set, signum)) return 0;

    if (sa.sa_handler != SIG_IGN && sa.sa_handler != SIG_DFL) {
        sa.sa_handler(signum);
        return 1;
    }
    return 0;
}

void runFocusMode(int numOfRounds, int duration) {
    struct sigaction sa1, sa2, sa3;

    // Initialize signal handlers
    sa1.sa_handler = handle_email;
    sigemptyset(&sa1.sa_mask);
    sa1.sa_flags = 0;
    sigaction(SIG_EMAIL, &sa1, NULL);

    sa2.sa_handler = handle_reminder;
    sigemptyset(&sa2.sa_mask);
    sa2.sa_flags = 0;
    sigaction(SIG_REMIND, &sa2, NULL);

    sa3.sa_handler = handle_doorbell;
    sigemptyset(&sa3.sa_mask);
    sa3.sa_flags = 0;
    sigaction(SIG_DB, &sa3, NULL);

    sigset_t actions;

    // First time: global header
    printf("Entering Focus Mode. All distractions are blocked.\n");

    for (int i = 0; i < numOfRounds; i++) {
        sigset_t pending;
        sigemptyset(&pending);
        sigemptyset(&actions);

        sigaddset(&actions, SIG_EMAIL);
        sigaddset(&actions, SIG_REMIND);
        sigaddset(&actions, SIG_DB);
        sigprocmask(SIG_BLOCK, &actions, NULL);

        // Round header
        printf("══════════════════════════════════════════════\n");
        printf("                Focus Round %d                \n", i + 1);
        printf("──────────────────────────────────────────────\n");

        char choice = '\0';
        for (int j = 0; j < duration; j++) {
            menu();
            printf(">> ");
            fflush(stdout);
            scanf(" %c", &choice);
            if (choice == 'q') {
                break; // Skip remaining iterations in this round
            }

            switch (choice) {
                case '1':
                    kill(getpid(), SIG_EMAIL);
                    break;
                case '2':
                    kill(getpid(), SIG_REMIND);
                    break;
                case '3':
                    kill(getpid(), SIG_DB);
                    break;
                default:
                    break;
            }
        }

        // End of round - check distractions
        printf("──────────────────────────────────────────────\n");
        printf("        Checking pending distractions...      \n");
        printf("──────────────────────────────────────────────\n");

        if (sigpending(&pending) == -1) {
            perror("sigpending");
            exit(EXIT_FAILURE);
        }

        int counter = 0;
        counter += check(pending, SIG_EMAIL, sa1);
        counter += check(pending, SIG_REMIND, sa2);
        counter += check(pending, SIG_DB, sa3);

        if (counter == 0) {
            printf("No distractions reached you this round.\n");
        }

        printf("──────────────────────────────────────────────\n");
        printf("             Back to Focus Mode.              \n");
        printf("══════════════════════════════════════════════\n");
    }

    // Final line after all rounds
    printf("Focus Mode complete. All distractions are now unblocked.\n");
}
