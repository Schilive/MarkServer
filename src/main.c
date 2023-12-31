/* This file is of the project MarkServer, which is under the MIT licence. For
 * more information, github.com/Schilive/MarkServer.
 *
 * This file was created originally in 2023/09/05.
 */

/*
 * This file contains the main function of MarkServer. Currently, it only
 * handles two options: "--version", and "--help" with aliases.
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#ifdef __linux__
#define TARGET_SYSTEM "Linux"
#else
#ifdef __unix__
#define TARGET_SYSTEM "Unix"
#else
#ifdef __WIN64
#define TARGET_SYSTEM "Win64"
#else
#ifdef __WIN32
#define TARGET_SYSTEM "Win32"
#else
#define TARGET_SYSTEM "N/A"
#endif
#endif
#endif
#endif

#define HAIL_MESSAGE "Save Markiplier!"

/* Prints a version message to 'out'. */
static void print_version_message(FILE *out)
{
        fprintf(out,
                "\n"
                "MarkServer version "
                VERSION
                " compiled on "
                "'" __DATE__ "'"
                " for "
                "'" TARGET_SYSTEM "'."
                "\n"
                "MarkServer is under the MIT licence. For more information, "
                "see github.com/Schilive/MarkServer."
                "\n"
                "MarkServer was built by <schilive100@gmail.com> as a fun "
                "learning project, to understand HTTP. And in honour of our "
                "loved and great MARKIPLIER! Our King!"
                "\n");
}

/* Prints the help message to 'out', using the name of the executable
 * 'exeName'. */
static void print_help_message(FILE *out, const char *exeName)
{
        fprintf(out,
                "MarkServer is under the MIT licence. For more "
                "information see github.com/Schilive/MarkServer."
                "\n"
                "MarkServer is a learning project, that implements an "
                "HTTP/1.0 server. Currently, it implements only a help "
                "message."
                "\n\n"
                "Usage: %s [options]"
                "\n\n"
                "Options:"
                "\n"
                "--help         Displays this message\n"
                "--version      Displays version information\n",
                exeName);
}

/* Returns whether a string is a help option, since the options has some
 * aliases to help find the help message. */
static bool is_help_option(const char *str)
{
        return  !strcmp(str, "--help")  ||      !strcmp(str, "-help") ||
                !strcmp(str, "/help")   ||      !strcmp(str, "/?");
}

/* Parses and handles the options passes by the command-line. Returns a logical
 * exit code for the outcome of the operations taken. */
static int handle_options(int argc, char **argv)
{
        bool help = false;
        bool vers = false;

        for (int i = 1; i < argc; i++) {
                if (!strcmp(argv[i], "--version"))
                        vers = true;
                else if (is_help_option(argv[i]))
                        help = true;
        }

        if (help) {
                print_help_message(stdout, argv[0]);
                return 0;
        } if (vers) {
                print_version_message(stdout);
                return 0;
        }

        printf("MarkServer. For help, please use '%s --help'\n", argv[0]);

        return 1;
}

int main(int argc, char **argv)
{
        printf(HAIL_MESSAGE "\n");
        return handle_options(argc, argv);
}
