
#include <stdio.h>
#include "cr.h"

CR_EXPORT int cr_main(cr_plugin *ctx, cr_op operation) {

    switch (operation) {
        case CR_LOAD:
            printf("loading");
            return 0;
        case CR_UNLOAD:
            printf("unloading");
            // if needed, save stuff to pass over to next instance
            return 0;
        case CR_CLOSE:
            printf("closing");
            return 0;
        case CR_STEP:
            return 0;
    }

    return 0;
}

