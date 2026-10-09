#include "common.h"
#include "parser.h"
#include "runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CMLL_VERSION "1.0.0"

static void usage(
    const char *program
)
{
    printf(
        "CMLL %s\n"
        "Command Language Made Legible\n"
        "\n"
        "Usage:\n"
        "  %s <file.cmll>\n"
        "  %s --version\n"
        "  %s --help\n",
        CMLL_VERSION,
        program,
        program,
        program
    );
}

int main(
    int argc,
    char **argv
)
{
    char *source;
    CMLLParser parser;
    CMLLNode *program;
    CMLLRuntime runtime;

    if (argc != 2) {
        usage(argv[0]);
        return 1;
    }

    if (
        strcmp(
            argv[1],
            "--version"
        ) == 0
    ) {
        printf(
            "CMLL %s\n",
            CMLL_VERSION
        );

        return 0;
    }

    if (
        strcmp(
            argv[1],
            "--help"
        ) == 0
    ) {
        usage(argv[0]);
        return 0;
    }

    source =
        cmll_read_file(
            argv[1],
            NULL
        );

    if (!source) {
        fprintf(
            stderr,
            "CMLL: cannot open '%s'\n",
            argv[1]
        );

        return 1;
    }

    cmll_parser_init(
        &parser,
        source,
        argv[1]
    );

    program =
        cmll_parse(&parser);

    if (!program) {
        free(source);
        return 1;
    }

    cmll_runtime_init(
        &runtime,
        program,
        argv[1],
        source
    );

    {
        int result;

        result =
            cmll_runtime_execute(
                &runtime
            );

        cmll_runtime_free(
            &runtime
        );

        cmll_node_free(
            program
        );

        free(source);

        return result;
    }
}
